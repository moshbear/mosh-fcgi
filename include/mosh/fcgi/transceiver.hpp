//! \file transceiver.hpp Defines the Fastcgippm0sh::Transceiver class
/***************************************************************************
* Copyright (C) 2007 Eddie                                                 *
*                                                                          *
* This file is part of fastcgi++.                                          *
*                                                                          *
* fastcgi++ is free software: you can redistribute it and/or modify it     *
* under the terms of the GnU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOuT *
* ANY WaRRANtY; without even the implied warranty of MERCHaNTABILItY or    *
* FITNEsS FoR A PaRTICULAR PURPOsE.  See the GnU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GnU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/


#ifndef MOSH_FCGI_TRANSCEIVER_HPP
#define MOSH_FCGI_TRANSCEIVER_HPP

#include <map>
#include <list>
#include <queue>
#include <algorithm>
#include <map>
#include <vector>
#include <memory>

extern "C" {
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
}

#include <mosh/fcgi/bits/array_deleter.hpp>
#include <mosh/fcgi/bits/block.hpp>
#include <mosh/fcgi/protocol/header.hpp>
#include <mosh/fcgi/protocol/message.hpp>
#include <mosh/fcgi/exceptions.hpp>

MOSH_FCGI_BEGIN

//! Handles low level communication with "the other side"
/*!
 * This class handles the sending/receiving/buffering of data through the OS level sockets and also
 * the creation/destruction of the sockets themselves.
 */
class Transceiver
{
public:
	//! General transceiver handler
	/*!
	 * This function is called by Manager::handler() to both transmit data passed to it from
	 * requests and relay received data back to them as a Message. The function will return true
	 * if there is nothing at all for it to do.
	 *
	 * @return Boolean value indicating whether there is data to be transmitted or received
	 */
	bool handler();

	//! Direct interface to Buffer::request_write()
	Block request_write(size_t size) {
		return buffer.request_write(size);
	}
	//! Direct interface to Buffer::secure_write()
	void secure_write(size_t size, protocol::Full_id id, bool kill) {
		buffer.secure_write(size, id, kill);
		transmit();
	}
	//! Constructor
	/*!
	 * Construct a transceiver object based on an initial file descriptor to listen on and
	 * a function to pass messages on to.
	 *
	 * @param[in] fd File descriptor to listen for connections on
	 * @param[in] send_message Function to call to pass messages to requests
	 */
	Transceiver(int fd, std::function<void(protocol::Full_id, protocol::Message)> send_message);
	//! Blocks until there is data to receive or a call to wake() is made
	void sleep() {
		poll(&poll_fds.front(), poll_fds.size(), -1);
	}

	//! Forces a wakeup from a call to sleep()
	void wake() {
		char x;
		ssize_t _unused = write(wakeup_fd_out, &x, 1);
		_unused = _unused;
	}

private:
	//! %Buffer type for receiving FastCGI records
	struct Fd_buffer {
		//! Buffer for header information
		protocol::Header header_buffer;
		//! Buffer of complete Message
		protocol::Message message_buffer;
	};

	//! %Buffer type for transmission of FastCgI records
	/*!
	 * This buffer is implemented as a circle of Chunk objects; the number of which can grow and shrink as needed. Write
	 * space is requested with requestWrite() which thereby returns a Block which may be smaller
	 * than requested. The write is committed by calling secure_write(). A smaller space can be
	 * committed than was given to write on.
	 *
	 * All data written to the buffer has an associated file descriptor through which it
	 * is flushed. File descriptor association with data is managed through a queue of Frame
	 * objects.
	 */
	class Buffer
	{
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
			std::shared_ptr<char, Array_deleter<char>> data;
			//! Pointer to the first write byte in the chunk or 1+ the last read byte
			char* end;
			//! Creates a new data chunk
			Chunk(): data(new char[size]), end(data.get()) { }
			~Chunk() { }
			//! Creates a new object that shares the data of the old one
			Chunk(const Chunk& chunk): data(chunk.data), end(data.get()) { }
		};

		//! A list of chunks. Can contain from 2-infinity
		std::list<Chunk> chunks;
		//! Iterator pointing to the chunk currently used for writing
		std::list<Chunk>::iterator write_it;

		//! Current read spot in the buffer
		char* p_read;

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
			Send_block(const char* data, size_t size, int fd): data(data), size(size), fd(fd) { }
			//! Create a new object that shares the data of the old
			Send_block(const Send_block& send_block): data(send_block.data), size(send_block.size), fd(send_block.fd) { }
			//! Pointer to the first byte in the block
			const char* data;
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

	//! %Buffer for transmitting data
	Buffer buffer;
	//! Function to call to pass messages to requests
	std::function<void(protocol::Full_id, protocol::Message)> send_message;

	//! poll() file descriptors container
	std::vector<pollfd> poll_fds;
	//! Socket to listen for connections on
	int socket;
	//! Input file descriptor to the wakeup socket pair
	int wakeup_fd_in;
	//! Output file descriptor to the wakeup socket pair
	int wakeup_fd_out;

	//! Container associating file descriptors with their receive buffers
	std::map<int, Fd_buffer> fd_buffers;

	//! Transmit all buffered data possible
	int transmit();
};

MOSH_FCGI_END

#endif
