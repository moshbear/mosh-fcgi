//! @file protocol.cpp Implementation for protocol/*
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

extern "C" {
#include <netinet/in.h>
#include <arpa/inet.h>
}

#include <cstdint>
#include <cstring>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/funcs.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/bits/aligned.hpp>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {

const std::map<std::string, std::string> management_params = {
		{ "FCGI_MAX_CONNS", "10" }, 
		{ "FCGI_MAX_REQS",  "50" },
		{ "FCGI_MPXS_CONNS", "1" }
	};

MOSH_FCGI::u_string make_param_header(std::pair<std::string, std::string> const& param) {
	MOSH_FCGI::u_string buf;
	uint32_t nbuf;
	size_t off = 0;

	size_t name_size = param.first.size();
	size_t value_size = param.second.size();

	if (name_size >= 0x80000000UL || value_size >= 0x80000000UL)
		throw std::invalid_argument("Parameter strings too large");	
	
	buf.resize(((name_size >= 0x80) ? 4 : 1) + ((value_size >= 0x80) ? 4 : 1) + name_size + value_size);
	
	if (name_size >= 0x80) {
		nbuf = htonl(name_size);
		memcpy(&buf[0], &nbuf, 4);
		buf[0] |= 0x80;
		off += 4;
	} else {
		buf[0] = name_size;
		off += 1;
	}
	if (value_size >= 0x80) {
		nbuf = htonl(value_size);
		memcpy(&buf[off], &nbuf, 4);
		buf[off] |= 0x80;
		off += 4;
	} else {
		buf[off] = value_size;
		off += 1;
	}
	memcpy(&buf[off], param.first.data(), name_size);
	off += name_size;
	memcpy(&buf[off], param.second.data(), value_size);
	return buf;
}
	
}

MOSH_FCGI_BEGIN

namespace protocol {

ssize_t process_param_header(const uchar* data, size_t data_size, std::pair<std::string, std::string>& result)
{
	size_t name_size;
	size_t value_size;

	size_t _size = data_size;

	if (data_size < 1)
		goto err_len;
	if (*data & 0x80) {
		if (data_size < 4)
			goto err_len;
		name_size = ntohl((aligned<4, uint32_t>(static_cast<const void*>(data)))) & 0x7FFFFFFFUL;
		data += 4;
		data_size -= 4;
	} else {
		name_size = zerofill_aligned_as<size_t, size_t, uchar>(data++);
		--data_size;
	}
	if (data_size < 1)
		goto err_len;
	if (*data & 0x80) {
		if (data_size < 4)
			goto err_len;
		value_size = ntohl((aligned<4, uint32_t>(static_cast<const void*>(data)))) & 0x7FFFFFFFUL;
		data += 4;
		data_size -= 4;
	} else {
		value_size = zerofill_aligned_as<size_t, size_t, uchar>(data++);
		--data_size;
	}

	if (name_size + value_size <= data_size) {
		result.first = std::string(reinterpret_cast<const char*>(data), name_size);
		data += name_size;
		data_size -= name_size;
		result.second = std::string(reinterpret_cast<const char*>(data), value_size);
		data += value_size;
		data_size -= value_size;
		return _size - data_size;	
	}
err_len:
	return -1;
}

u_string process_gv(const uchar* data, size_t data_len) {
	std::queue<std::string> queue;

	while (data_len > 0) {
		std::pair<std::string, std::string> params;
		ssize_t to_erase = process_param_header(data, data_len, params);
		if (to_erase == -1)
			break;
		data += to_erase;
		data_len -= to_erase;
		queue.push(std::move(params.first));
	}
	
	u_string res;

	while (!queue.empty()) {
		std::map<std::string, std::string>::const_iterator p = management_params.find(queue.front());
		if (p != management_params.end()) 
			res += make_param_header(*p);
		queue.pop();
	}
	return res;

}

const char* record_type_labels[] = {
	"INVALID",
	"BEGIN_REQUEST",
	"ABORT_REQUEST",
	"END_REQUEST",
	"PARAMS",
	"IN",
	"OUT",
	"ERR",
	"DATA",
	"GET_VALUES",
	"GET_VALUES_RESULT",
	"UNKNOWN_TYPE"
};

}

MOSH_FCGI_END
