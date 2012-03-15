#include <map>
#include <memory>
#include <string>
#include <utility>
#include <cassert>
extern "C" {
#include <sys/types.h>
#include <unistd.h>
}

#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/protocol/funcs.hpp>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/protocol/begin_request.hpp>
#include <mosh/fcgi/protocol/full_id.hpp> 
#include <mosh/fcgi/protocol/message.hpp>
#include <mosh/fcgi/protocol/unknown_type.hpp>
#include <mosh/fcgi/transceiver.hpp>
#include <mosh/fcgi/bits/locked.hpp>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/protocol.hpp>
#include <src/u.hpp>
#include <src/namespace.hpp>

namespace {

/*! @brief The global multiplexer 
 *
 *  When signals are caught, the pid is checked to see which manager instance is responsible.
 *  If there isn't one, then the signal is ignored. Otherwise, the instance's terminate (if SIGUSR1)
 *  or stop (if SIGTERM) function is called.
 *  
 *  @see signal_handler
 */
MOSH_FCGI::Rw_locked<std::map<pid_t, MOSH_FCGI::Manager*>> instances;

/*! @brief A list of recognized management parameters.
 *
 * Used by process_gv().
 */
const std::map<std::string, std::string> management_params = {
		{ "FCGI_MAX_CONNS", "10" }, 
		{ "FCGI_MAX_REQS",  "50" },
		{ "FCGI_MPXS_CONNS", "1" }
	};

//! Process a GET_VALUES record and generate an appropriate output
SRC::u_string process_gv(const SRC::uchar* data, size_t data_len) {
	// enqueue params
	std::queue<std::string> queue;
	while (data_len > 0) {
		std::pair<std::string, std::string> params;
		ssize_t to_erase = MOSH_FCGI::protocol::process_param_record(data, data_len, params);
		if (to_erase == -1)
			break;
		data += to_erase;
		data_len -= to_erase;
		queue.push(std::move(params.first));
	}
	// dequeue params
	SRC::u_string res;
	while (!queue.empty()) {
		std::map<std::string, std::string>::const_iterator p = management_params.find(queue.front());
		if (p != management_params.end()) 
			res += SRC::make_param_record(*p);
		queue.pop();
	}
	return res;
}

//! Global signal handler
void signal_handler(int signo) {
	pid_t pid = getpid();
	std::lock_guard<MOSH_FCGI::Rw_lock> rd_lock(instances);
	auto inst = instances.find(pid);
	if (inst == instances.end())
		return;
	assert (inst->second != nullptr);
	switch (signo) {
	case SIGUSR1:
		inst->second->terminate();
		break;
	case SIGTERM:
		inst->second->stop();
		break;
	}
}
	
void setup_signals() {
	struct sigaction _sa;
	_sa.sa_handler = signal_handler;

	sigaction(SIGPIPE, &_sa, NULL); // No-op
	sigaction(SIGUSR1, &_sa, NULL); // [this]->terminate()
	sigaction(SIGTERM, &_sa, NULL); // [this]->stop()
}

}


MOSH_FCGI_BEGIN

Manager::Manager(int fd, std::function<Request_base*()> new_req)
	: transceiver(fd, [&] (protocol::Full_id a1, protocol::Message a2) {
					push(a1, a2);
				}),
	new_request(new_req), asleep(false), do_stop(false), do_terminate(false)
{
	setup_signals();

	// Register our pid & instance ptr in the global table
	std::lock_guard<Rw_lock> instance_lk(instances);
	pid_t pid = getpid();
	assert(instances.find(pid) == instances.end());
	instances.upgrade_lock();
	instances.insert({ pid, this });
}

Manager::~Manager() {
	// Unregister our pid & instance from the global table
	pid_t pid = getpid();
	std::lock_guard<Rw_lock> instance_lk(instances);
	auto m = instances.find(pid);
	assert((m != instances.end()) && (m->second == this));
	instances.upgrade_lock();
	instances.erase(m);
}

void Manager::push(protocol::Full_id id, protocol::Message message) {
	using namespace std;
	using namespace protocol;

	if (id.fcgi_id) {
		std::lock_guard<Rw_lock> read_lock(requests);
		auto it = requests.find(id);
		if (it != requests.end()) {
			lock_guard<mutex> mes_lock(it->second->messages);
			it->second->messages.push(message);
			lock_guard<mutex> tasks_guard(tasks);
			tasks.push(id);
		} else if (!message.type) {
			aligned<8, Header> _header(static_cast<const void *>(message.data.get()));
			Header& header = _header;
			if (header.type() == Record_type::begin_request) {
				aligned<8, Begin_request> _body(static_cast<const void *>(message.data.get() + sizeof(Header)));
				Begin_request& body = _body;
				requests.upgrade_lock();
				std::shared_ptr<Request_base>& request = requests[id];
				request.reset(new_request());
				request->set(id, transceiver, body.role(), !body.keep_conn(),
						[&] (protocol::Message a1) {
							this->push(id, a1);
						}
				);
			} else {
				return;
			}
		}
	} else {
		messages.push(message);
		tasks.push(id);
	}

	lock_guard<mutex> sleep_guard(asleep);
	if (asleep)
		transceiver.wake();
}

void Manager::handler() {
	thread_id = pthread_self();
	for (;;) {
		{
			std::lock_guard<std::mutex> stop_guard(do_stop);
			if (do_stop) {
				do_stop = false;
				return;
			}
		}

		bool sleep = transceiver.handler();

		{
			std::lock_guard<std::mutex> terminate_guard(do_terminate);
			if (do_terminate) {
				std::lock_guard<Rw_lock> req_lock(requests);
				bool req_empty = requests.empty();
				if (req_empty && sleep) {
					do_terminate = false;
					return;
				}
			}
		}
		
		tasks.lock();
		asleep.lock();

		if (tasks.empty()) {
			tasks.unlock();
			asleep = true;
			asleep.unlock();
			if (sleep)
				transceiver.sleep();
			asleep.lock();
			asleep = false;
			asleep.unlock();
			continue;
		}

		asleep.unlock();

		protocol::Full_id id = tasks.front();
		tasks.pop();
		tasks.unlock();
		if (id.fcgi_id == 0)
			local_handler(id);
		else {
			std::lock_guard<Rw_lock> read_lock(requests);
			auto it = requests.find(id);
			if (it != requests.end() && it->second->handler()) {
				requests.upgrade_lock();
				requests.erase(it);
			}
		}
	}
}


void Manager::local_handler(protocol::Full_id id) {
	using namespace protocol;
	using namespace std;
	Message msg(messages.front());
	messages.pop();
	if (msg.type)
		return;
	aligned<8, Header> _header(static_cast<const void *>(msg.data.get()));
	Header& header = _header;
	switch (header.type()) {
	case Record_type::get_values: {
		u_string res = process_gv(msg.data.get() + sizeof(Header), header.content_length());
		Block buffer(transceiver.request_write(res.size() + 16));
		memcpy(buffer.data + 8, res.data(), res.size());
		Header h(version, Record_type::get_values_result, 0, res.size(), (8 - (res.size() % 8)) % 8);
		memcpy(buffer.data, &h, sizeof(Header));
		transceiver.secure_write(sizeof h + h.content_length() + h.padding_length(), id, false);
	}; break;
	default: {
		Block buffer(transceiver.request_write(sizeof(Header) + sizeof(Unknown_type)));
		Header send_header(version, Record_type::unknown_type, 0, sizeof(Unknown_type), 0);
		Unknown_type send_body;
		send_body.type() = header.type();
		memcpy(buffer.data, &send_header, sizeof(Header));
		memcpy(buffer.data + sizeof(Header), &send_body, sizeof(Unknown_type));
		transceiver.secure_write(sizeof(Header) + sizeof(Unknown_type), id, false);
	}; break;
	}
}

void Manager::terminate() {
	std::lock_guard<std::mutex> lock(do_terminate);
	do_terminate = true;
}

void Manager::stop() {
	std::lock_guard<std::mutex> lock(do_stop);
	do_stop = true;
}

MOSH_FCGI_END
