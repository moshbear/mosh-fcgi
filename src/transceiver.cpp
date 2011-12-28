//! @file transceiver.cpp Defines member functions for Transceiver
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
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

#include <functional>
#include <vector>
#include <limits>

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

//! Predicate for comparing the file descriptor of a pollfd
struct equals_fd : public std::unary_function<pollfd, bool> {
	int fd;
	explicit equals_fd(int fd): fd(fd) {}
	bool operator()(const pollfd& x) const {
		return x.fd == fd;
	};
};

MOSH_FCGI_BEGIN

int Transceiver::transmit() {
	for(;;) {
		Buffer::Send_block send_block(buffer.request_read());
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
			buffer.free_read(sent);
			assert (send_block.size <= std::numeric_limits<ssize_t>::max()); 
			if (sent != send_block.size)
				break;
		} else
			break;
	}
	return buffer.empty();
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
		actual = read(fd, reinterpret_cast<char*>(&header_buffer) + message_buffer.size,
					sizeof(Header) - message_buffer.size);
		if (actual < 0 && errno != EAGAIN)
			throw exceptions::Socket_read(fd, errno);
		if (actual > 0)
			message_buffer.size += actual;
		if (message_buffer.size != sizeof(Header)) {
			return (transmit_empty);
		}

		message_buffer.data.reset(new char[sizeof(Header)
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
	actual = read(fd, static_cast<char*>(message_buffer.data.get()) + message_buffer.size, needed);
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
	: buffer(poll_fds, fd_buffers), send_message(send_message_), poll_fds(2), socket(fd_)  {
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

MOSH_FCGI_END
