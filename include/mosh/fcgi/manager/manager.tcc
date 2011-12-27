//! @file mosh/fcgi/manager/manager.tcc Template implementation of fcgi::Manager
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
#include <mutex>
#include <memory>
#include <utility>

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
	std::lock_guard<std::mutex> lock(terminate_lock);
	do_terminate = true;
}

template<class T>
void Manager<T>::stop() {
	std::lock_guard<std::mutex> lock(stop_lock);
	do_stop = true;
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

	if (id.fcgi_id) {
		std::lock_guard<Rw_lock> read_lock(requests_lock);
		auto it = requests.find(id);
		if (it != requests.end()) {
			lock_guard<mutex> mes_lock(it->second->messages_lock);
			it->second->messages.push(message);
			lock_guard<mutex> tasks_guard(tasks_lock);
			tasks.push(id);
		} else if (!message.type) {
			aligned<8, Header> _header(static_cast<const void *>(message.data.get()));
			Header& header = _header;
			if (header.type() == Record_type::begin_request) {
				aligned<8, Begin_request> _body(static_cast<const void *>(message.data.get() + sizeof(Header)));
				Begin_request& body = _body;
				requests_lock.upgrade_lock();
				std::shared_ptr<T>& request = requests[id];
				request.reset(new T);
				request->set(id, transceiver, body.role(), !body.keep_conn(),
				             std::bind(&Manager::push, std::ref(*this), id, stdph::_1));
			} else {
				return;
			}
		}
	} else {
		messages.push(message);
		tasks.push(id);
	}

	lock_guard<mutex> sleep_guard(sleep_lock);
	if (asleep)
		transceiver.wake();
}

template<class T>
void Manager<T>::handler() {
	thread_id = pthread_self();

	for (;;) {
		{
			std::lock_guard<std::mutex> stop_guard(stop_lock);
			if (do_stop) {
				do_stop = false;
				return;
			}
		}

		bool sleep = transceiver.handler();

		{
			std::lock_guard<std::mutex> terminate_guard(terminate_lock);
			if (do_terminate) {
				std::lock_guard<Rw_lock> read_lock(requests_lock);
				bool req_empty = requests.empty();
				if (req_empty && sleep) {
					do_terminate = false;
					return;
				}
			}
		}
		
		tasks_lock.lock();
		sleep_lock.lock();

		if (tasks.empty()) {
			tasks_lock.unlock();
			asleep = true;
			sleep_lock.unlock();
			if (sleep)
				transceiver.sleep();
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
			std::lock_guard<Rw_lock> read_lock(requests_lock);
			auto it = requests.find(id);
			if (it != requests.end() && it->second->handler()) {
				requests_lock.upgrade_lock();
				requests.erase(it);
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
		aligned<8, Header> _header(static_cast<const void *>(message.data.get()));
		Header& header = _header;
		switch (header.type()) {
		case Record_type::get_values: {
			size_t name_size;
			size_t value_size;
			const char* name;
			const char* value;
			process_param_header(message.data.get() + sizeof(Header), header.content_length(),
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
			
			Header send_header(version, Record_type::unknown_type, 0, sizeof(Unknown_type), 0);
			Unknown_type send_body;
			send_body.type() = header.type();

			memcpy(buffer.data, &send_header, sizeof(Header));
			memcpy(buffer.data + sizeof(Header), &send_body, sizeof(Unknown_type));

			transceiver.secure_write(sizeof(Header) + sizeof(Unknown_type), id, false);

			break;
		}
		}
	}
}

MOSH_FCGI_END

#endif
