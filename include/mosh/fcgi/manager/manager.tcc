//! \file manager.hpp Defines the Fcgi::Manager class
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


#ifndef MOSH_FCGI_MANAGER_MANAGER_TCC
#define MOSH_FCGI_MANAGER_MANAGER_TCC

#include <map>
#include <queue>
#include <algorithm>
#include <cstring>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>

extern "C" {
#include <signal.h>
#include <pthread.h>
}

#include <mosh/fcgi/exceptions.hpp>
#include <mosh/fcgi/bits/block.hpp>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/protocol/funcs.hpp>
#include <mosh/fcgi/protocol/full_id.hpp> 
#include <mosh/fcgi/protocol/header.hpp>
#include <mosh/fcgi/protocol/begin_request.hpp>
#include <mosh/fcgi/protocol/unknown_type.hpp>
#include <mosh/fcgi/protocol/management_reply.hpp>
#include <mosh/fcgi/protocol/message.hpp>
#include <mosh/fcgi/transceiver.hpp>
#include <mosh/fcgi/manager/manager.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

template<class T> Manager<T>* Manager<T>::instance = 0;

template<class T>
void Manager<T>::terminate() {
	boost::lock_guard<boost::mutex> lock(terminate_mutex);
	terminate_bool = true;
}

template<class T>
void Manager<T>::stop() {
	boost::lock_guard<boost::mutex> lock(stop_mutex);
	stop_bool = true;
}

template<class T>
void Manager<T>::signal_handler(int signum) {

	switch (signum) {
	case SIGUSR1:
		if (instance)
			instance->terminate();
		break;
	case SIGTERM:
		if (instance)
			instance->stop();
		break;
	}
}

template<class T>
void Manager<T>::setup_signals() {
	struct sigaction _sa;
	_sa.sa_handler = Manager<T>::signal_handler;

	sigaction(SIGPIPE, &_sa, NULL);
	sigaction(SIGUSR1, &_sa, NULL);
	sigaction(SIGTERM, &_sa, NULL);
}

template<class T>
void Manager<T>::push(protocol::Full_id id, protocol::Message message) {
	using namespace std;
	using namespace protocol;
	using namespace boost;

	if (id.fcgi_id) {
		upgrade_lock<shared_mutex> req_lock(requests);
		typename Requests::iterator it(requests.find(id));
		if (it != requests.end()) {
			lock_guard<mutex> mes_lock(it->second->messages);
			it->second->messages.push(message);
			lock_guard<mutex> tasks_lock(tasks);
			tasks.push(id);
		} else if (!message.type) {
			Header& header = *(Header*)message.data.get();
			if (header.get_type() == Record_type::begin_request) {
				Begin_request& body = *(Begin_request*)(message.data.get() + sizeof(Header));
				upgrade_to_unique_lock<shared_mutex> lock(req_lock);
				boost::shared_ptr<T>& request = requests[id];
				request.reset(new T);
				request->set(id, transceiver, body.get_role(), !body.get_keep_conn(),
				             bind(&Manager::push, boost::ref(*this), id, _1));
			} else
				return;
		}
	} else {
		messages.push(message);
		tasks.push(id);
	}

	lock_guard<mutex> sleep_lock(sleep_mutex);
	if (asleep)
		transceiver.wake();
}

template<class T>
void Manager<T>::handler() {
	using namespace std;
	using namespace boost;

	thread_id = pthread_self();

	while (1) {{
			{
				lock_guard<mutex> stop_lock(stop_mutex);
				if (stop_bool) {
					stop_bool = false;
					return;
				}
			}

			bool sleep = transceiver.handler();

			{
				lock_guard<mutex> terminate_lock(terminate_mutex);
				if (terminate_bool) {
					shared_lock<shared_mutex> requests_lock(requests);
					if (requests.empty() && sleep) {
						terminate_bool = false;
						return;
					}
				}
			}

			unique_lock<mutex> tasks_lock(tasks);
			unique_lock<mutex> sleep_lock(sleep_mutex);

			if (tasks.empty()) {
				tasks_lock.unlock();

				asleep = true;
				sleep_lock.unlock();

				if (sleep) transceiver.sleep();

				sleep_lock.lock();
				asleep = false;
				sleep_lock.unlock();

				continue;
			}

			sleep_lock.unlock();

			protocol::Full_id id = tasks.front();
			tasks.pop();
			tasks_lock.unlock();

			if (id.fcgi_id == 0)
				local_handler(id);
			else {
				upgrade_lock<shared_mutex> req_read_lock(requests);
				typename map<protocol::Full_id, boost::shared_ptr<T> >::iterator it(requests.find(id));
				if (it != requests.end() && it->second->handler()) {
					upgrade_to_unique_lock<shared_mutex> req_write_lock(req_read_lock);
					requests.erase(it);
				}
			}
		}
	}
}

template<class T>
void Manager<T>::local_handler(protocol::Full_id id) {
	using namespace std;
	using namespace protocol;
	Message message(messages.front());
	messages.pop();

	if (!message.type) {
		const Header& header = *(Header*)message.data.get();
		switch (header.get_type()) {
		case Record_type::get_values: {
			size_t name_size;
			size_t value_size;
			const char* name;
			const char* value;
			process_param_header(message.data.get() + sizeof(Header), header.get_content_length(),
			                   name, name_size, value, value_size);
			if (name_size == 14 && !memcmp(name, "FCGI_MAX_CONNS", 14)) {
				Block buffer(transceiver.request_write(sizeof(max_conns_reply)));
				memcpy(buffer.data, (const char*)&max_conns_reply, sizeof(max_conns_reply));
				transceiver.secure_write(sizeof(max_conns_reply), id, false);
			} else if (name_size == 13 && !memcmp(name, "FCGI_MAX_REQS", 13)) {
				Block buffer(transceiver.request_write(sizeof(max_reqs_reply)));
				memcpy(buffer.data, (const char*)&max_reqs_reply, sizeof(max_reqs_reply));
				transceiver.secure_write(sizeof(max_reqs_reply), id, false);
			} else if (name_size == 15 && !memcmp(name, "FCGI_MPXS_CONNS", 15)) {
				Block buffer(transceiver.request_write(sizeof(mpxs_conns_reply)));
				memcpy(buffer.data, (const char*)&mpxs_conns_reply, sizeof(mpxs_conns_reply));
				transceiver.secure_write(sizeof(mpxs_conns_reply), id, false);
			}

			break;
		}

		default: {
			Block buffer(transceiver.request_write(sizeof(Header) + sizeof(Unknown_type)));

			Header& send_header = *(Header*)buffer.data;
			send_header.set_version(version);
			send_header.set_type(Record_type::unknown_type);
			send_header.set_request_id(0);
			send_header.set_content_length(sizeof(Unknown_type));
			send_header.set_padding_length(0);

			Unknown_type& send_body = *(Unknown_type*)(buffer.data + sizeof(Header));
			send_body.set_type(header.get_type());

			transceiver.secure_write(sizeof(Header) + sizeof(Unknown_type), id, false);

			break;
		}
		}
	}
}

MOSH_FCGI_END

#endif
