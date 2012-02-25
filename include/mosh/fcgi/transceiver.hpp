//! @file  mosh/fcgi/transceiver.hpp Defines the MOSH_FCGI::Transceiver class
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


#ifndef MOSH_FCGI_TRANSCEIVER_HPP
#define MOSH_FCGI_TRANSCEIVER_HPP

#include <map>
#include <memory>
#include <vector>
extern "C" {
#include <poll.h>
}

#include <mosh/fcgi/bits/block.hpp>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/protocol/header.hpp>
#include <mosh/fcgi/protocol/message.hpp>
#include <mosh/fcgi/exceptions.hpp>

MOSH_FCGI_BEGIN

//! Handles low level communication with "the other side"
/*!
 * This class handles the sending/receiving/buffering of data through the OS level sockets and also
 * the creation/destruction of the sockets themselves.
 */
class Transceiver {
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

	//! Interface to Buffer::request_write()
	Block request_write(size_t size);
	//! Interface to Buffer::secure_write()
	void secure_write(size_t size, protocol::Full_id id, bool kill);
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
	void sleep();

	//! Forces a wakeup from a call to sleep()
	void wake();

private:
	//! %Buffer type for receiving FastCGI records
	struct Fd_buffer {
		//! Buffer for header information
		protocol::Header header_buffer;
		//! Buffer of complete Message
		protocol::Message message_buffer;
	};

	//! %Buffer type for transmission of FastCGI records
	class Buffer;

	//! %Buffer for transmitting data
	std::unique_ptr<Buffer> pbuf;
	
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
