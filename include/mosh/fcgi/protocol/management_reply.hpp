//! @file protocol/management_reply.hpp FastCGI GET_VALUES_RESULT record
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
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


#ifndef MOSH_FCGI_PROTOCOL_HPP
#define MOSH_FCGI_PROTOCOL_HPP

#include <cstdint>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/protocol/header.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace protocol {

#pragma pack(push, 1)

	/*! @brief Used for the reply of FastCGI management records of type get_values
	 * 
	 * This class template is an efficient tool for replying to get_values management
	 * records. The structure represents a complete record (body+header) of a name-value pair to be
	 * sent as a reply to a management value query. The templating allows the structure
	 * to be exactly the size that is needed so it can be casted to raw data and transmitted
	 * as is. Note that the name and value lengths are left as single bytes so they are limited
	 * in range from 0-127.
	 *
	 * @tparam name_len Length of name in bytes (0-127). Null terminator not included.
	 * @tparam value_len Length of value in bytes (0-127). Null terminator not included.
	 * @tparam padding_len Length of padding at the end of the record. This is needed to keep
	 * the record size a multiple of chunkSize.
	 */
	template<uint8_t name_len, uint8_t value_len, uint8_t padding_len>
	struct Management_reply {
	private:
		//! Management records header
		Header header;
		//! Length in bytes of name
		uint8_t name_length;
		//! Length in bytes of value
		uint8_t value_length;
		//! Name data
		uint8_t name[name_len];
		//! Value data
		uint8_t value[value_len];
		//! Padding data
		uint8_t padding[padding_len];
	public:
		/*! @brief  Construct the record based on the name data and value data
		 *
		 * A full record is constructed from the name-value data. After
		 * construction the structure can be casted to raw data and transmitted
		 * as is. The size of the data arrays pointed to by name and value are
		 * assumed to correspond with the name_len and padding_len template
		 * parameters passed to the class.
		 *
		 * @param[in] name Pointer to name data
		 * @param[in] value Pointer to value data
		 */
		Management_reply(const char* name, const char* value)
		: name_length(name_len), value_length(value_len) {
			for(int i = 0; i < name_len; i++)
				this->name[i] = *(name+i);
			for(int i = 0; i < value_len; i++)
				this->value[i] = *(value+i);
			header.version() = version;
			header.type() = Record_type::get_values_result;
			header.request_id() = 0;
			header.content_length() = name_len + value_len;
			header.padding_length() = padding_len;
		}
	};

#pragma pack(pop)

}

MOSH_FCGI_END

#endif
