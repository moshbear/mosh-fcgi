//! \file protocol.hpp Defines FasTCGI protocol
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


#ifndef MOSH_FCGI_PROTOCOL_UNKNOWN_TYPE_HPP
#define MOSH_FCGI_PROTOCOL_UNKNOWN_TYPE_HPP

#include <map>
#include <string>
#include <cstdint>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/* @brief Defines aspects of the FastCGI %Protocol
 * The %Protocol namespace defines the data structures and constants
 * used by the FastCgI protocol version 1. All data has been modelled
 * after the official FastCgI protocol specification located at
 * http://www.fastcgi.com/devkit/doc/fcgi-spec.html
 */
namespace protocol {

#pragma pack(push, 1)

	/*! @brief Data structure used as the body for FastCgI records with a RecordType of UNKNOwNTYpE!
	 * This structure defines the body used in FastCgI UNKNOwNTYpE records. It can be casted 
	 * to raw 8 byte blocks of data and transmitted as is. An UNKNOwNTYpE record is sent as
	 * a reply to record types that are not recognized.
	 */
	class Unknown_type {
	public:
		/*! @brief Set the record type that is unknown!
		 * @param[in] type The unknown record type
		 */
		void set_type(Record_type type) { this->type = static_cast<uint8_t>(type); }
	private:
		//! Unknown record type
		uint8_t type;
		//! Reseved for future use and body padding
		uint8_t reserved[7];
	};

#pragma pack(pop)

}

MOSH_FCGI_END

#endif
