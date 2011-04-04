//! \file transceiver.hpp Defines the Fastcgipp_m0sh::Transceiver class
/***************************************************************************
* Copyright (C) 2007 Eddie                                                 *
*                                                                          *
* This file is part of fastcgi++.                                          *
*                                                                          *
* fastcgi++ is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/


#ifndef TRANSCEIVER_HPP
#define TRANSCEIVER_HPP

#include <map>
#include <list>
#include <queue>
#include <algorithm>
#include <map>
#include <vector>
#include <functional>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_array.hpp>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include <fastcgipp-mosh/protocol.hpp>
#include <fastcgipp-mosh/exceptions.hpp>

//! Topmost namespace for the fastcgi++ library
namespace Fastcgipp_m0sh
{
//! A raw block of memory
/*!
 * The purpose of this structure is to communicate a block of data to be written to
 * a Transceiver::Buffer
 */
struct Block {
	//! Construct from a pointer and size
	/*!
	 * @param[in] data_ Pointer to start of memory location
	 * @param[in] size_ Size in bytes of memory location
	 */
	Block(char* data_, size_t size_): data(data_), size(size_) { }
	//! Copies pointer and size, not data
	Block(const Block& block): data(block.data), size(block.size) { }
	Block(Block& block) {
		data = block.data;
		size = block.size;
	}
	//! Copies pointer and size, not data
	Block& operator=(const Block& block) {
		if (this != &block) {
			data = block.data;
			size = block.size;
		}
		return *this;
	}
	//! Pointer to start of memory location
	char* data;
	//! Size in bytes of memory location
	size_t size;
};

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

	//! Direct interface to Buffer::requestWrite()
	Block requestWrite(size_t size) {
		return buffer.requestWrite(size);
	}
	//! Direct interface to Buffer::secureWrite()
	void secureWrite(size_t size, Protocol::FullId id, bool kill) {
		buffer.secureWrite(size, id, kill);
		transmit();
	}
	//! Constructor
	/*!
	 * Construct a transceiver object based on an initial file descriptor to listen on and
	 * a function to pass messages on to.
	 *
	 * @param[in] fd_ File descriptor to listen for connections on
	 * @param[in] sendMessage_ Function to call to pass messages to requests
	 */
	Transceiver(int fd_, boost::function<void(Protocol::FullId, Message)> sendMessage_);
	//! Blocks until there is data to receive or a call to wake() is made
	void sleep() {
		poll(&pollFds.front(), pollFds.size(), -1);
	}

	//! Forces a wakeup from a call to sleep()
	void wake();

private:
	//! %Buffer type for receiving FastCGI records
	struct fdBuffer {
		//! Buffer for header information
		Protocol::Header headerBuffer;
		//! Buffer of complete Message
		Message messageBuffer;
	};

	//! %Buffer type for transmission of FastCGI records
	/*!
	 * This buffer is implemented as a circle of Chunk objects; the number of which can grow and shrink as needed. Write
	 * space is requested with requestWrite() which thereby returns a Block which may be smaller
	 * than requested. The write is committed by calling secureWrite(). A smaller space can be
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
			 * @param[in] size_ Size of the frame
			 * @param[in] closeFd_ Boolean value indication whether or not the file descriptor should be closed when the frame has been flushed
			 * @param[in] id_ Complete ID of the request making the frame
			 */
			Frame(size_t size_, bool closeFd_, Protocol::FullId id_)
				: size(size_), closeFd(closeFd_), id(id_)
			{ }
			//! Size of the frame
			size_t size;
			//! Boolean value indication whether or not the file descriptor should be closed when the frame has been flushed
			bool closeFd;
			//! Complete ID (contains a file descriptor) of associated with the data frame
			Protocol::FullId id;
		};
		//! Queue of frames waiting to be transmitted
		std::queue<Frame> frames;
		//! Minimum Block size value that can be returned from requestWrite()
		const static unsigned int minBlockSize = 256;
		//! A reference to Transceiver::pollFds for removing file descriptors when they are closed
		std::vector<pollfd>& pollFds;
		//! A reference to Transceiver::fdBuffer for deleting buffers upon closing of the file descriptor
		std::map<int, fdBuffer>& fdBuffers;
		//! %Chunk of data in Buffer
		struct Chunk {
			//! Size of data section of the chunk
			const static unsigned int size = 131072;
			//! Pointer to the first byte in the chunk data
			boost::shared_array<char> data;
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
		std::list<Chunk>::iterator writeIt;

		//! Current read spot in the buffer
		char* pRead;

	public:
		//! Constructor
		/*!
		 * @param[out] pollFds_ A reference to Transceiver::pollFds is needed for removing file descriptors when they are closed
		 * @param[out] fdBuffers_ A reference to Transceiver::fdBuffer is needed for deleting buffers upon closing of the file descriptor
		 */
		Buffer(std::vector<pollfd>& pollFds, std::map<int, fdBuffer>& fdBuffers_)
			: pollFds(pollFds), fdBuffers(fdBuffers_), chunks(1), writeIt(chunks.begin()), pRead(chunks.begin()->data.get())
		{ }

		//! Request a write block in the buffer
		/*!
		 * @param[in] size Requested size of write block
		 * @return Block of writable memory. Size may be less than requested
		 */
		Block requestWrite(size_t size) {
			return Block(writeIt->end, std::min(size, (size_t)(writeIt->data.get() + Chunk::size - writeIt->end)));
		}
		//! Secure a write in the buffer
		/*!
		 * @param[in] size Amount of bytes to secure
		 * @param[in] id Associated complete ID (contains file descriptor)
		 * @param[in] kill Boolean value indicating whether or not the file descriptor should be closed after transmission
		 */
		void secureWrite(size_t size, Protocol::FullId id, bool kill);

		//! %Block of memory for extraction from Buffer
		struct SendBlock {
			//! Constructor
			/*!
			 * @param[in] data_ Pointer to the first byte in the block
			 * @param[in] size_ Size in bytes of the data
			 * @param[in] fd_ File descriptor the data should be written to
			 */
			SendBlock(const char* data_, size_t size_, int fd_): data(data_), size(size_), fd(fd_) { }
			//! Create a new object that shares the data of the old
			SendBlock(const SendBlock& sendBlock): data(sendBlock.data), size(sendBlock.size), fd(sendBlock.fd) { }
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
		SendBlock requestRead() {
			return SendBlock(pRead, frames.empty() ? 0 : frames.front().size, frames.empty() ? -1 : frames.front().id.fd);
		}
		//! Mark data in the buffer as transmitted and free it's memory
		/*!
		 * @param size Amount of bytes to mark as transmitted and free
		 */
		void freeRead(size_t size);

		//! Test of the buffer is empty
		/*!
		 * @return true if the buffer is empty
		 */
		bool empty() {
			return pRead == writeIt->end;
		}
	};

	//! %Buffer for transmitting data
	Buffer buffer;
	//! Function to call to pass messages to requests
	boost::function<void(Protocol::FullId, Message)> sendMessage;

	//! poll() file descriptors container
	std::vector<pollfd> pollFds;
	//! Socket to listen for connections on
	int socket;
	//! Input file descriptor to the wakeup socket pair
	int wakeUpFdIn;
	//! Output file descriptor to the wakeup socket pair
	int wakeUpFdOut;

	//! Container associating file descriptors with their receive buffers
	std::map<int, fdBuffer> fdBuffers;

	//! Transmit all buffered data possible
	int transmit();
};

//! Predicate for comparing the file descriptor of a pollfd
struct equalsFd : public std::unary_function<pollfd, bool> {
	int fd;
	explicit equalsFd(int fd_): fd(fd_) {}
	bool operator()(const pollfd& x) const {
		return x.fd == fd;
	};
};

//! Predicate for testing if the revents in a pollfd is non-zero
inline bool reventsZero(const pollfd& x)
{
	return x.revents;
}
}

#endif
