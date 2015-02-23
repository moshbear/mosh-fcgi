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

#include <queue>
#include <map>
#include <string>
#include <mutex>
#include <functional>

#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/funcs.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/protocol/end_request.hpp>
#include <mosh/fcgi/protocol/message.hpp>
#include <mosh/fcgi/exceptions.hpp>
#include <mosh/fcgi/bits/aligned.hpp>
#include <mosh/fcgi/bits/block.hpp>
#include <mosh/fcgi/transceiver.hpp>
#include <mosh/fcgi/fcgistream.hpp>
#include <mosh/fcgi/http/session.hpp>
#include <mosh/fcgi/bits/locked.hpp>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {

}

MOSH_FCGI_BEGIN

Request_base::Request_base()
: state(protocol::Record_type::params) {
	out.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	err.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
}

Request_base::~Request_base() {
}

bool Request_base::handler() {
	using namespace protocol;
	using namespace std;

	try {
		{
			std::lock_guard<std::mutex> lock(messages);
			message = messages.front();
			messages.pop();
		}
		if (message.type != 0) {
			if (response()) {
				complete(0);
				return true;
			}
			return false;
		}
		
		aligned<sizeof(Header), Header> _header(static_cast<const void*>(message.data.get()));
		Header& header = _header;
		const uchar* body = message.data.get() + sizeof(Header);
		switch (header.type()) {
		case Record_type::params: {
			if (state != Record_type::params)
				throw exceptions::Record_out_of_order(id, state, Record_type::params);
			if (header.content_length() == 0) {
				fill_params(true);
				if (role == Role::authorizer) {
					state = Record_type::out;
					if (response()) {
						complete(0);
						return true;
					}
					break;
				}
				state = Record_type::in;
				break;
			}
			pbuf.append(body, body + header.content_length());
			fill_params(false);
		} break;
		case Record_type::in: {
			if (state != Record_type::in)
				throw exceptions::Record_out_of_order(id, state, Record_type::in);
			if (header.content_length() == 0) {
				in_handler(nullptr, 0);
				if (role == Role::filter) {
					state = Record_type::data;
					break;
				}
				state = Record_type::out;
				if (response()) {
					complete(0);
					return true;
				}
				break;
			}
			in_handler(body, header.content_length());
		} break;
		case Record_type::data: {
			if (state != Record_type::data)
				throw exceptions::Record_out_of_order(id, state, Record_type::data);
			if (header.content_length() == 0) {
				data_handler(nullptr, 0);
				state = Record_type::out;
				if (response()) {
					complete(0);
					return true;
				}
			}
			data_handler(body, header.content_length());
		} break;
		case Record_type::abort_request:
				return true;
		default:;
		}
	} catch (std::exception& e) {
		err << e.what() << std::endl;
		complete(1);
		return true;
	}
	return false;	
}

void Request_base::complete(int app_status) {
	using namespace protocol;
	out.flush();
	err.flush();

	Header hdr(version, Record_type::end_request, id.fcgi_id, sizeof(End_request), 0);
	End_request ereq(app_status, Protocol_status::request_complete);

	Block buffer(transceiver->request_write(sizeof(Header) + sizeof(End_request)));

	memcpy(buffer.data, &hdr, sizeof(Header));
	memcpy(buffer.data + sizeof(Header), &ereq, sizeof(End_request));

	transceiver->secure_write(sizeof(Header) + sizeof(End_request), id, kill_con);
}

void Request_base::set(protocol::Full_id id, Transceiver& transceiver, protocol::Role role, bool kill_con,
			std::function<void(protocol::Message)> callback) {
	this->kill_con = kill_con;
	this->id = id;
	this->transceiver = &transceiver;
	this->role = role;
	this->callback = callback;

	err.set(id, transceiver, protocol::Record_type::err);
	out.set(id, transceiver, protocol::Record_type::out);
}

void Request_base::fill_params(bool term) {
	while (pbuf.size()) {
		std::pair<std::string, std::string> params;
		if (!term) {
			ssize_t to_erase = protocol::process_param_record(pbuf.data(), pbuf.size(), params);
			if (to_erase == -1)
				return;
			pbuf.erase(0, to_erase);
		}
		if (params_handler(params) && !term)
			envs.insert(params);

	}
}

u_string Request_base::dump() const {
	std::basic_stringstream<uchar> ss;
	ss << "Transceiver: " << "\r\n";
	ss << "Role: " << ([=](protocol::Role r) {
				switch (r) {
				case protocol::Role::invalid: return "<invalid>";
				case protocol::Role::responder: return "RESPONDER";
				case protocol::Role::authorizer: return "AUTHORIZER";
				case protocol::Role::filter: return "FILTER";
				default: return "?????";
				}
			})(role) << "\r\n";
	ss << " Conn fd: " << id.fd << "\r\n";
	ss << " Request id: " << id.fcgi_id << "\r\n";
	ss << " Kill con: " << kill_con << "\r\n";
	return ss.str();
}

			
MOSH_FCGI_END
