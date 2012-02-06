//! @file protocol.cpp Implementation for protocol/*
/***************************************************************************
* Copyright (C) 2011 m0shbear
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
#include <stdexcept>
#include <string>
#include <utility>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/funcs.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/protocol/management_reply.hpp>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace protocol {

ssize_t process_param_header(const uchar* data, size_t data_size, std::pair<std::string, std::string>>& result)
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
		result.first = std::string(data, name_size);
		data += name_size;
		data_size -= name_size;
		result.second = std::string(data, value_size);
		data += value_size;
		data_size -= value_size;
		return _size - data_size;	
	}
err_len:
	return -1;
}

Management_reply<14, 2, 8> max_conns_reply("FCGI_MAX_CONNS", "10");
Management_reply<13, 2, 1> max_reqs_reply("FCGI_MAX_REQS", "50");
Management_reply<15, 1, 8> mpxs_conns_reply("FCGI_MPXS_CONNS", "1");

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
