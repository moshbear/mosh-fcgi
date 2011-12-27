//! \file protocol.cpp Defines Fast_cGI protocol
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

extern "C" {
#include <netinet/in.h>
#include <arpa/inet.h>
}

#include <cstdint>
#include <cstring>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/funcs.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/protocol/management_reply.hpp>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace protocol {

bool process_param_header(const char* data, size_t data_size,
        const char*& name, size_t& name_size,
        const char*& value, size_t& value_size)
{
	if (*data & 0x80) {
		name_size = ntohl((aligned<4, uint32_t>(static_cast<const void*>(data)))) & 0x7FFFFFFF;
		data += sizeof(uint32_t);
	} else
		name_size = zerofill_aligned_as<size_t, size_t, char>(data++);

	if (*data & 0x80) {
		value_size = ntohl((aligned<4, uint32_t>(static_cast<const void*>(data)))) & 0x7FFFFFFF;
		data += sizeof(uint32_t);
	} else
		value_size = zerofill_aligned_as<size_t, size_t, char>(data++);

	name = data;
	value = name + name_size;
	return (name + name_size + value_size <= data + data_size);
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
