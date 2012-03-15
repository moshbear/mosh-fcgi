//! @file  mosh/fcgi/manager.hpp Defines the fcgi::Manager class
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


#ifndef MOSH_FCGI_MANAGER_HPP
#define MOSH_FCGI_MANAGER_HPP

#include <map>
#include <string>
#include <queue>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <cstring>

#include <signal.h>
#include <pthread.h>

#include <mosh/fcgi/exceptions.hpp>
#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/protocol/message.hpp>
#include <mosh/fcgi/transceiver.hpp>
#include <mosh/fcgi/bits/locked.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace stdph = std::placeholders;

//! General task and protocol management class
/*!
 * Handles all task and protocol management, creation/destruction
 * of requests and passing of messages to requests. The template argument
 * should be a class type derived from the Request class with at least the
 * response() function defined. To operate this class all that needs to be
 * done is creating an object and calling handler() on it.
 *
 * @tparam T Class that will handle individual requests. Should be derived from
 * the Request class.
 */
class Manager {
	
public:
	//! Construct from a file descriptor
	/*!
	 * The only piece of data required to construct a %Manager object is a
	 * file descriptor to listen on for incoming connections. By default
	 * modfastcgi sets up file descriptor 0 to do this so it is the value
	 * passed by default to the constructor. The only time it would be another
	 * value is if an external FastCGI server was defined.
	 *
	 * @param[in] fd File descriptor to listen on.
	 * @param new_req New request handler
	 */
	Manager(int fd = 0, std::function<Request_base*()> new_req = []() -> Request_base* { throw std::invalid_argument("Attempt to instantiate Request_base"); });
	virtual ~Manager();

	//! General handling function to be called after construction
	/*!
	 * This function will loop continuously manager tasks and FastCGI
	 * requests until either the stop() function is called (obviously from another
	 * thread) or the appropriate signals are caught.
	 *
	 * @sa setup_signals()
	 */
	void handler();

	//! Halter for the handler() function
	/*!
	 * This function is intended to be called from a thread separate from the handler()
	 * in order to halt it. It should also be called by a signal handler in the case of
	 * of a SIGTERM. Once %handler() has been halted it may be re-called to pick up
	 * exactly where it left off without any data loss.
	 *
	 * @sa setup_signals()
	 * @sa signal_handler()
	 */
	void stop();

	//! Terminator for the handler() function
	/*!
	 * This function is intended to be called from  a signal handler in the case of
	 * of a SIGUSR1. It is similar to stop() except that handler() will wait until
	 * all requests are complete before halting.
	 *
	 * @sa setup_signals()
	 * @sa signal_handler()
	 */
	void terminate();
	
	//! Passes messages to requests
	/*!
	 * Whenever a message needs to be passed to a request, it must be done through
	 * this function. %Requests are associated with their protocol::Full_id value so
	 * that and the message itself is all that is needed. Calling this function from
	 * another thread is safe. Although this function can be called from outside
	 * the mosh-fcgi library, the Request class contains a callback function based
	 * on this that is more usable. An id with a protocol::Request_id of 0 means the message
	 * is destined for the %Manager itself. Should a message by passed with an id that doesn't
	 * exist, it will be discarded.
	 *
	 * @param[in] id The id of the request the message should go to
	 * @param[in] message The message itself
	 *
	 * @sa Request::callback
	 */
	void push(protocol::Full_id id, protocol::Message message);

private:
	//! Handles low level communication with the other side
	Transceiver transceiver;
	
	//! Queue for pending tasks
	Mutexed<std::queue<protocol::Full_id>> tasks;
	
	//! A queue of messages for the manager itself
	Mutexed<std::queue<protocol::Message>> messages;

	/*! @brief Associative container for active requests
	 *
	 * This container associated the protocol::Full_id of each active request with a pointer
	 * to the actual Request object.
	 */
	Rw_locked<std::map<protocol::Full_id, std::shared_ptr<Request_base>>> requests;

	//! Handler for new requests
	std::function<Request_base* ()> new_request;
	
	//! Handles management messages
	/*!
	 * This function is called by handler() in the case that a management message is recieved.
	 * Although the request id of a management record is always 0, the protocol::Full_id associated
	 * with the message is passed to this function to keep track of it's associated
	 * file descriptor.
	 *
	 * @param[in] id Full_id associated with the messsage.
	 */
	void local_handler(protocol::Full_id id);

	//! Indicated whether or not the manager is currently in sleep mode
	Mutexed_0<bool> asleep;

	/*! @brief The pthread id of the thread the handler() function is operating in.
	 *
	 * Although this library is intended to be used with boost::thread and not pthread, the underlying
	 * pthread id of the %handler() function is needed to call pthreadkill() when sleep is to be interrupted.
	 */
	pthread_t thread_id;
	//! Boolean value indicating that handler() should halt
	/*!
	 * @sa stop()
	 */
	Mutexed_0<bool> do_stop;
	//! Boolean value indication that handler() should terminate
	/*!
	 * @sa terminate()
	 */
	Mutexed_0<bool> do_terminate;
};

/*! @brief A templated derivative of Manager
 *
 *  This is a templated derivative class of %Manager that fills the new_req function with
 *  @c new @c T.
 *
 *  @tparam T Request handling class; must be derived from Request_base
 */
template <typename T>
class ManagerT : public std::enable_if<std::is_base_of<Request_base, T>::value, Manager>::type {
	friend class Manager;
public:
	ManagerT(int fd = 0) : Manager(fd, [](){ return new T; }) { }
	virtual ~ManagerT() { }
protected:
		
private:
};


MOSH_FCGI_END

#endif
