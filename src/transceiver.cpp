//! @file transceiver.cpp Defines member functions for Transceiver
/***************************************************************************
* Copyright (C) 2011-2 m0shbear                                            *
*               2007 Eddie                                                 *
*                                                                          *
* This file is part of mosh-fcgi.                                          *
*                                                                          *
* mosh-fcgi is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* mosh-fcgi is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with mosh-fcgi.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/

#include <algorithm>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <vector>

extern "C" {
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>
}

#include <mosh/fcgi/exceptions.hpp>
#include <mosh/fcgi/bits/block.hpp>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/protocol/header.hpp>
#include <mosh/fcgi/protocol/message.hpp>
#include <mosh/fcgi/transceiver.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

#include <src/array_deleter.hpp>
#include <src/namespace.hpp>

namespace {

//! Predicate for comparing the file descriptor of a pollfd
struct equals_fd : public std::unary_function<pollfd, bool> {
	int fd;
	explicit equals_fd(int fd): fd(fd) {}
	bool operator()(const pollfd& x) const {
		return x.fd == fd;
	};
};

}

MOSH_FCGI_BEGIN

//! %Buffer type for transmission of FastCGI records
/*!
 * This buffer is implemented as a circle of Chunk objects; the number of which can grow and shrink as needed. Write
 * space is requested with request_write() which thereby returns a Block which may be smaller
 * than requested. The write is committed by calling secure_write(). A smaller space can be
 * committed than was given to write on.
 *
 * All data written to the buffer has an associated file descriptor through which it
 * is flushed. File descriptor association with data is managed through a queue of Frame
 * objects.
 */
class Transceiver::Buffer {
	//! %Frame of data associated with a file descriptor
	struct Frame {
		//! Constructor
		/*!
		 * @param[in] size Size of the frame
		 * @param[in] close_fd Boolean value indication whether or not the file descriptor should be closed when the frame has been flushed
		 * @param[in] id Complete ID of the request making the frame
		 */
		Frame(size_t size, bool close_fd, protocol::Full_id id)
			: size(size), close_fd(close_fd), id(id)
		{ }
		//! Size of the frame
		size_t size;
		//! Boolean value indication whether or not the file descriptor should be closed when the frame has been flushed
		bool close_fd;
		//! Complete ID (contains a file descriptor) of associated with the data frame
		protocol::Full_id id;
	};
	//! Queue of frames waiting to be transmitted
	std::queue<Frame> frames;
	//! Minimum Block size value that can be returned from request_write()
	const static unsigned int min_block_size = 256;
	//! A reference to Transceiver::poll_fds for removing file descriptors when they are closed
	std::vector<pollfd>& poll_fds;
	//! A reference to Transceiver::Fd_buffer for deleting buffers upon closing of the file descriptor
	std::map<int, Fd_buffer>& fd_buffers;
	//! %Chunk of data in Buffer
	struct Chunk {
		//! Size of data section of the chunk
		const static unsigned int size = 131072;
		//! Pointer to the first byte in the chunk data
		std::shared_ptr<uchar> data;
		//! Pointer to the first write byte in the chunk or 1+ the last read byte
		uchar* end;
		//! Creates a new data chunk
		Chunk(): data(new uchar[size], SRC::Array_deleter<uchar>()), end(data.get()) { }
		~Chunk() { }
		//! Creates a new object that shares the data of the old one
		Chunk(const Chunk& chunk): data(chunk.data), end(data.get()) { }
	};
	//! A list of chunks. Can contain from 2-infinity
	std::list<Chunk> chunks;
	//! Iterator pointing to the chunk currently used for writing
	std::list<Chunk>::iterator write_it;
	//! Current read spot in the buffer
	uchar* p_read;
public:
	//! Constructor
	/*!
	 * @param[out] poll_fds A reference to Transceiver::poll_fds is needed for removing file descriptors when they are closed
	 * @param[out] fd_buffers A reference to Transceiver::Fd_buffer is needed for deleting buffers upon closing of the file descriptor
	 */
	Buffer(std::vector<pollfd>& poll_fds, std::map<int, Fd_buffer>& fd_buffers)
		: poll_fds(poll_fds), fd_buffers(fd_buffers), chunks(1), write_it(chunks.begin()), p_read(chunks.begin()->data.get())
	{ }
	//! Request a write block in the buffer
	/*!
	 * @param[in] size Requested size of write block
	 * @return Block of writable memory. Size may be less than requested
	 */
	Block request_write(size_t size) {
		return Block(write_it->end, std::min(size, (size_t)(write_it->data.get() + Chunk::size - write_it->end)));
	}
	//! Secure a write in the buffer
	/*!
	 * @param[in] size Amount of bytes to secure
	 * @param[in] id Associated complete ID (contains file descriptor)
	 * @param[in] kill Boolean value indicating whether or not the file descriptor should be closed after transmission
	 */
	void secure_write(size_t size, protocol::Full_id id, bool kill);
		//! %Block of memory for extraction from Buffer
	struct Send_block {
		//! Constructor
		/*!
		 * @param[in] data Pointer to the first byte in the block
		 * @param[in] size Size in bytes of the data
		 * @param[in] fd File descriptor the data should be written to
		 */
		Send_block(const uchar* data, size_t size, int fd): data(data), size(size), fd(fd) { }
		//! Create a new object that shares the data of the old
		Send_block(const Send_block& send_block): data(send_block.data), size(send_block.size), fd(send_block.fd) { }
		//! Pointer to the first byte in the block
		const uchar* data;
		//! Size in bytes of the data
		size_t size;
		//! File descriptor the data should be written to
		int fd;
	};
	//! Request a block of data for transmitting
	/*!
	 * @return A block of data with a file descriptor to transmit it out
	 */
	Send_block request_read() {
		return Send_block(p_read, frames.empty() ? 0 : frames.front().size, frames.empty() ? -1 : frames.front().id.fd);
	}
	//! Mark data in the buffer as transmitted and free it's memory
	/*!
	 * @param size Amount of bytes to mark as transmitted and free
	 */
	void free_read(size_t size);
	//! Test of the buffer is empty
	/*!
	 * @return true if the buffer is empty
	 */
	bool empty() {
		return p_read == write_it->end;
	}
};

Block Transceiver::request_write(size_t size) {
	return pbuf->request_write(size);
}

void Transceiver::secure_write(size_t size, protocol::Full_id id, bool kill) {
	pbuf->secure_write(size, id, kill);
	transmit();
}

void Transceiver::sleep() {
	poll(&poll_fds.front(), poll_fds.size(), -1);
}

void Transceiver::wake() {
	char x;
	ssize_t _unused = write(wakeup_fd_out, &x, 1);
	_unused = _unused;
}

int Transceiver::transmit() {
	for(;;) {
		Buffer::Send_block send_block(pbuf->request_read());
		if (send_block.size) {
			ssize_t sent = write(send_block.fd, send_block.data, send_block.size);
			if (sent < 0) {
				if (errno == EPIPE) {
					poll_fds.erase(std::find_if(poll_fds.begin(), poll_fds.end(), equals_fd(send_block.fd)));
					fd_buffers.erase(send_block.fd);
					sent = send_block.size;
				} else if (errno != EAGAIN)
					throw exceptions::Socket_write(send_block.fd, errno);
			}
			pbuf->free_read(sent);
			assert (send_block.size <= std::numeric_limits<ssize_t>::max()); 
			if (sent != send_block.size)
				break;
		} else
			break;
	}
	return pbuf->empty();
}

void Transceiver::Buffer::secure_write(size_t size, protocol::Full_id id, bool kill) {
	write_it->end += size;
	if (min_block_size > (write_it->data.get() + Chunk::size - write_it->end) && ++write_it == chunks.end()) {
		chunks.push_back(Chunk());
		--write_it;
	}
	frames.push(Frame(size, kill, id));
}

bool Transceiver::handler() {
	using namespace std;
	using namespace protocol;

	bool transmit_empty = transmit();

	int ret_val = poll(&poll_fds.front(), poll_fds.size(), 0);
	if (ret_val == 0) {
		return (transmit_empty);
	}
	if (ret_val < 0)
		throw exceptions::Poll(errno);

	vector<pollfd>::iterator poll_fd = find_if(poll_fds.begin(), poll_fds.end(),
							[](const pollfd& x) { return !!(x.revents); });

	if (poll_fd->revents & POLLHUP) {
		fd_buffers.erase(poll_fd->fd);
		poll_fds.erase(poll_fd);
		return false;
	}

	int fd = poll_fd->fd;
	if (fd == socket) {
		sockaddr_un addr;
		socklen_t addrlen = sizeof(sockaddr_un);
		fd = accept(fd, reinterpret_cast<sockaddr*>(&addr), &addrlen);
		fcntl(fd, F_SETFL, (fcntl(fd, F_GETFL) | O_NONBLOCK) ^ O_NONBLOCK);

		poll_fds.push_back(pollfd());
		poll_fds.back().fd = fd;
		poll_fds.back().events = POLLIN | POLLHUP;

		Message& message_buffer = fd_buffers[fd].message_buffer;
		message_buffer.size = 0;
		message_buffer.type = 0;
	} else if (fd == wakeup_fd_in) {
		char x;
		ssize_t r = read(wakeup_fd_in, &x, 1);
		r = r;
		return false;
	}

	Message& message_buffer = fd_buffers[fd].message_buffer;
	Header& header_buffer = fd_buffers[fd].header_buffer;

	ssize_t actual;
	// Are we in the process of recieving some part of a frame?
	if (!message_buffer.data) {
		// Are we recieving a partial header or new?
		actual = read(fd, reinterpret_cast<uchar*>(&header_buffer) + message_buffer.size,
					sizeof(Header) - message_buffer.size);
		if (actual < 0 && errno != EAGAIN)
			throw exceptions::Socket_read(fd, errno);
		if (actual > 0)
			message_buffer.size += actual;
		if (message_buffer.size != sizeof(Header)) {
			return (transmit_empty);
		}

		message_buffer.data.reset(new uchar[sizeof(Header)
						   + header_buffer.content_length()
						   + header_buffer.padding_length()
						  ]);
		// we can't make assumptions about message_buffer.data's alignment, so memcpy() is used to safely copy the POD
		memcpy(static_cast<void*>(message_buffer.data.get()), static_cast<const void*>(&header_buffer), sizeof(Header));
	}
	aligned<8, Header> _header(static_cast<void*>(message_buffer.data.get()));
	Header& header = _header;
	size_t needed = header.content_length() + header.padding_length() + sizeof(Header) - message_buffer.size;
	assert(needed <= std::numeric_limits<ssize_t>::max()); // Send a bug report if this assertion fails
	actual = read(fd, static_cast<uchar*>(message_buffer.data.get()) + message_buffer.size, needed);
	if (actual < 0 && errno != EAGAIN)
		throw exceptions::Socket_read(fd, errno);
	if (actual > 0)
		message_buffer.size += actual;

	// Did we recieve a full frame?
	if (actual == needed) {
		send_message(Full_id(header_buffer.request_id(), fd), message_buffer);
		message_buffer.size = 0;
		message_buffer.data.reset();
		return false;
	}
	return (transmit_empty);
}

void Transceiver::Buffer::free_read(size_t size) {
	p_read += size;
	if (p_read >= chunks.begin()->end) {
		if (write_it == chunks.begin()) {
			p_read = write_it->data.get();
			write_it->end = p_read;
		} else {
			if (write_it == --chunks.end()) {
				chunks.begin()->end = chunks.begin()->data.get();
				chunks.splice(chunks.end(), chunks, chunks.begin());
			} else
				chunks.pop_front();
			p_read = chunks.begin()->data.get();
		}
	}
	if ((frames.front().size -= size) == 0) {
		if (frames.front().close_fd) {
			poll_fds.erase(std::find_if(poll_fds.begin(), poll_fds.end(), equals_fd(frames.front().id.fd)));
			close(frames.front().id.fd);
			fd_buffers.erase(frames.front().id.fd);
		}
		frames.pop();
	}

}

Transceiver::Transceiver(int fd_, std::function<void(protocol::Full_id, protocol::Message)> send_message_)
	: pbuf(new Buffer(poll_fds, fd_buffers)), send_message(send_message_), poll_fds(2), socket(fd_)  {
	// Let's setup an in/out socket for waking up poll()
	int soc_pair[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, soc_pair);
	wakeup_fd_in = soc_pair[0];
	fcntl(wakeup_fd_in, F_SETFL, (fcntl(wakeup_fd_in, F_GETFL) | O_NONBLOCK) ^ O_NONBLOCK);
	wakeup_fd_out = soc_pair[1];

	fcntl(socket, F_SETFL, (fcntl(socket, F_GETFL) | O_NONBLOCK) ^ O_NONBLOCK);
	poll_fds[0].events = POLLIN | POLLHUP;
	poll_fds[0].fd = socket;
	poll_fds[1].events = POLLIN | POLLHUP;
	poll_fds[1].fd = wakeup_fd_in;
}

Transceiver::~Transceiver() { }

MOSH_FCGI_END
