//! \file manager.hpp Defines the Fastcgipp_m0sh::Manager class
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


#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <map>
#include <string>
#include <queue>
#include <algorithm>
#include <cstring>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <signal.h>
#include <pthread.h>

#include <fastcgipp-mosh/exceptions.hpp>
#include <fastcgipp-mosh/protocol.hpp>
#include <fastcgipp-mosh/transceiver.hpp>

//! Topmost namespace for the fastcgi++ library
namespace Fastcgipp_m0sh
{
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
template<typename T>
class Manager
{
public:
	//! Construct from a file descriptor
	/*!
	 * The only piece of data required to construct a %Manager object is a
	 * file descriptor to listen on for incoming connections. By default
	 * mod_fastcgi sets up file descriptor 0 to do this so it is the value
	 * passed by default to the constructor. The only time it would be another
	 * value is if an external FastCGI server was defined.
	 *
	 * @param [in] fd File descriptor to listen on.
	 */
	Manager(int fd = 0)
		: transceiver(fd, boost::bind(&Manager::push, boost::ref(*this), _1, _2)),
		  asleep(false), terminateBool(false), stopBool(false) {
		setupSignals();
		instance = this;
	}
	~Manager() {
		instance = 0;
	}

	//! General handling function to be called after construction
	/*!
	 * This function will loop continuously manager tasks and FastCGI
	 * requests until either the stop() function is called (obviously from another
	 * thread) or the appropriate signals are caught.
	 *
	 * @sa setupSignals()
	 */
	void handler();

	//! Passes messages to requests
	/*!
	 * Whenever a message needs to be passed to a request, it must be done through
	 * this function. %Requests are associated with their Protocol::FullId value so
	 * that and the message itself is all that is needed. Calling this function from
	 * another thread is safe. Although this function can be called from outside
	 * the fastcgi++ library, the Request class contains a callback function based
	 * on this that is more usable. An id with a Protocol::RequestId of 0 means the message
	 * is destined for the %Manager itself. Should a message by passed with an id that doesn't
	 * exist, it will be discarded.
	 *
	 * @param[in] id The id of the request the message should go to
	 * @param[in] message The message itself
	 *
	 * @sa Request::callback
	 */
	void push(Protocol::FullId id, Message message);

	//! Halter for the handler() function
	/*!
	 * This function is intended to be called from a thread separate from the handler()
	 * in order to halt it. It should also be called by a signal handler in the case of
	 * of a SIGTERM. Once %handler() has been halted it may be re-called to pick up
	 * exactly where it left off without any data loss.
	 *
	 * @sa setupSignals()
	 * @sa signalHandler()
	 */
	void stop();


	//! Configure the handlers for POSIX signals
	/*!
	 * By calling this function appropriate handlers will be set up for SIGPIPE, SIGUSR1 and
	 * SIGTERM. It is called by default upon construction of a Manager object. Should
	 * the user want to override these handlers, it should be done post-construction.
	 *
	 * @sa signalHandler()
	 */
	void setupSignals();
private:
	//! Handles low level communication with the other side
	Transceiver transceiver;

	//! Queue type for pending tasks
	/*!
	 * This is merely a derivation of a std::queue<Protocol::FullId> and a
	 * boost::mutex that gives data locking abilities to the STL container.
	 */
	class Tasks: public std::queue<Protocol::FullId>, public boost::mutex {};
	//! Queue for pending tasks
	/*!
	 * This contains a queue of Protocol::FullId that need their handlers called.
	 */
	Tasks tasks;

	//! Associative container type for active requests
	/*!
	 * This is merely a derivation of a std::map<Protocol::FullId, boost::shared_ptr<T> > and a
	 * boost::shared_mutex that gives data locking abilities to the STL container.
	 */
	class Requests : public std::map<Protocol::FullId, boost::shared_ptr<T> >, public boost::shared_mutex {};
	//! Associative container type for active requests
	/*!
	 * This container associated the Protocol::FullId of each active request with a pointer
	 * to the actual Request object.
	 */
	Requests requests;

	//! A queue of messages for the manager itself
	std::queue<Message> messages;

	//! Handles management messages
	/*!
	 * This function is called by handler() in the case that a management message is recieved.
	 * Although the request id of a management record is always 0, the Protocol::FullId associated
	 * with the message is passed to this function to keep track of it's associated
	 * file descriptor.
	 *
	 * @param[in] id FullId associated with the messsage.
	 */
	inline void localHandler(Protocol::FullId id);

	//! Indicated whether or not the manager is currently in sleep mode
	bool asleep;
	//! Mutex to make accessing asleep thread safe
	boost::mutex sleepMutex;
	//! The pthread id of the thread the handler() function is operating in.
	/*!
	 * Although this library is intended to be used with boost::thread and not pthread, the underlying
	 * pthread id of the %handler() function is needed to call pthread_kill() when sleep is to be interrupted.
	 */
	pthread_t threadId;

	//! Boolean value indicating that handler() should halt
	/*!
	 * @sa stop()
	 */
	bool stopBool;
	//! Mutex to make stopBool thread safe
	boost::mutex stopMutex;
	//! Boolean value indication that handler() should terminate
	/*!
	 * @sa terminate()
	 */
	bool terminateBool;
	//! Mutex to make terminateMutex thread safe
	boost::mutex terminateMutex;

	//! General function to handler POSIX signals
	static void signalHandler(int signum);
	//! Pointer to the %Manager object
	static Manager<T>* instance;
	//! Terminator for the handler() function
	/*!
	 * This function is intended to be called from  a signal handler in the case of
	 * of a SIGUSR1. It is similar to stop() except that handler() will wait until
	 * all requests are complete before halting.
	 *
	 * @sa setupSignals()
	 * @sa signalHandler()
	 */
	inline void terminate();
};
}

template<class T>
Fastcgipp_m0sh::Manager<T>* Fastcgipp_m0sh::Manager<T>::instance = 0;

template<class T>
void Fastcgipp_m0sh::Manager<T>::terminate()
{
	boost::lock_guard<boost::mutex> lock(terminateMutex);
	terminateBool = true;
}

template<class T>
void Fastcgipp_m0sh::Manager<T>::stop()
{
	boost::lock_guard<boost::mutex> lock(stopMutex);
	stopBool = true;
}

template<class T>
void Fastcgipp_m0sh::Manager<T>::signalHandler(int signum)
{
	switch (signum) {
	case SIGUSR1: {
		if (instance) instance->terminate();
		break;
	}
	case SIGTERM: {
		if (instance) instance->stop();
		break;
	}
	}
}

template<class T>
void Fastcgipp_m0sh::Manager<T>::setupSignals()
{
	struct sigaction sigAction;
	sigAction.sa_handler = Fastcgipp_m0sh::Manager<T>::signalHandler;

	sigaction(SIGPIPE, &sigAction, NULL);
	sigaction(SIGUSR1, &sigAction, NULL);
	sigaction(SIGTERM, &sigAction, NULL);
}

template<class T>
void Fastcgipp_m0sh::Manager<T>::push(Protocol::FullId id, Message message)
{
	using namespace std;
	using namespace Protocol;
	using namespace boost;

	if (id.fcgiId) {
		upgrade_lock<shared_mutex> reqLock(requests);
		typename Requests::iterator it(requests.find(id));
		if (it != requests.end()) {
			lock_guard<mutex> mesLock(it->second->messages);
			it->second->messages.push(message);
			lock_guard<mutex> tasksLock(tasks);
			tasks.push(id);
		} else if (!message.type) {
			Header& header = *(Header*)message.data.get();
			if (header.getType() == BEGIN_REQUEST) {
				BeginRequest& body = *(BeginRequest*)(message.data.get() + sizeof(Header));
				upgrade_to_unique_lock<shared_mutex> lock(reqLock);
				shared_ptr<T>& request = requests[id];
				request.reset(new T);
				request->set(id, transceiver, body.getRole(), !body.getKeepConn(),
				             bind(&Manager::push, boost::ref(*this), id, _1));
			} else
				return;
		}
	} else {
		messages.push(message);
		tasks.push(id);
	}

	lock_guard<mutex> sleepLock(sleepMutex);
	if (asleep)
		transceiver.wake();
}

template<class T>
void Fastcgipp_m0sh::Manager<T>::handler()
{
	using namespace std;
	using namespace boost;

	threadId = pthread_self();

	while (1) {{
			{
				lock_guard<mutex> stopLock(stopMutex);
				if (stopBool) {
					stopBool = false;
					return;
				}
			}

			bool sleep = transceiver.handler();

			{
				lock_guard<mutex> terminateLock(terminateMutex);
				if (terminateBool) {
					shared_lock<shared_mutex> requestsLock(requests);
					if (requests.empty() && sleep) {
						terminateBool = false;
						return;
					}
				}
			}

			unique_lock<mutex> tasksLock(tasks);
			unique_lock<mutex> sleepLock(sleepMutex);

			if (tasks.empty()) {
				tasksLock.unlock();

				asleep = true;
				sleepLock.unlock();

				if (sleep) transceiver.sleep();

				sleepLock.lock();
				asleep = false;
				sleepLock.unlock();

				continue;
			}

			sleepLock.unlock();

			Protocol::FullId id = tasks.front();
			tasks.pop();
			tasksLock.unlock();

			if (id.fcgiId == 0)
				localHandler(id);
			else {
				upgrade_lock<shared_mutex> reqReadLock(requests);
				typename map<Protocol::FullId, boost::shared_ptr<T> >::iterator it(requests.find(id));
				if (it != requests.end() && it->second->handler()) {
					upgrade_to_unique_lock<shared_mutex> reqWriteLock(reqReadLock);
					requests.erase(it);
				}
			}
		}
	}
}

template<class T>
void Fastcgipp_m0sh::Manager<T>::localHandler(Protocol::FullId id)
{
	using namespace std;
	using namespace Protocol;
	Message message(messages.front());
	messages.pop();

	if (!message.type) {
		const Header& header = *(Header*)message.data.get();
		switch (header.getType()) {
		case GET_VALUES: {
			size_t nameSize;
			size_t valueSize;
			const char* name;
			const char* value;
			processParamHeader(message.data.get() + sizeof(Header), header.getContentLength(),
			                   name, nameSize, value, valueSize);
			if (nameSize == 14 && !memcmp(name, "FCGI_MAX_CONNS", 14)) {
				Block buffer(transceiver.requestWrite(sizeof(maxConnsReply)));
				memcpy(buffer.data, (const char*)&maxConnsReply, sizeof(maxConnsReply));
				transceiver.secureWrite(sizeof(maxConnsReply), id, false);
			} else if (nameSize == 13 && !memcmp(name, "FCGI_MAX_REQS", 13)) {
				Block buffer(transceiver.requestWrite(sizeof(maxReqsReply)));
				memcpy(buffer.data, (const char*)&maxReqsReply, sizeof(maxReqsReply));
				transceiver.secureWrite(sizeof(maxReqsReply), id, false);
			} else if (nameSize == 15 && !memcmp(name, "FCGI_MPXS_CONNS", 15)) {
				Block buffer(transceiver.requestWrite(sizeof(mpxsConnsReply)));
				memcpy(buffer.data, (const char*)&mpxsConnsReply, sizeof(mpxsConnsReply));
				transceiver.secureWrite(sizeof(mpxsConnsReply), id, false);
			}

			break;
		}

		default: {
			Block buffer(transceiver.requestWrite(sizeof(Header) + sizeof(UnknownType)));

			Header& sendHeader = *(Header*)buffer.data;
			sendHeader.setVersion(version);
			sendHeader.setType(UNKNOWN_TYPE);
			sendHeader.setRequestId(0);
			sendHeader.setContentLength(sizeof(UnknownType));
			sendHeader.setPaddingLength(0);

			UnknownType& sendBody = *(UnknownType*)(buffer.data + sizeof(Header));
			sendBody.setType(header.getType());

			transceiver.secureWrite(sizeof(Header) + sizeof(UnknownType), id, false);

			break;
		}
		}
	}
}

#endif
