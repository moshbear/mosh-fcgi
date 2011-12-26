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


#ifndef MOSH_FCGI_PROTOCOL_HEADER_HPP
#define MOSH_FCGI_PROTOCOL_HEADER_HPP

extern "C" {            //
#include <netinet/in.h> // for ntohs, ntohl
#include <arpa/inet.h>  //
}                       //
#include <cstdint>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/* @brief Defines aspects of the FastCGI %Protocol
 * The %Protocol namespace defines the data structures and constants
 * used by the FastCGI protocol version 1. All data has been modelled
 * after the official FastCGI protocol specification located at
 * http://www.fastcgi.com/devkit/doc/fcgi-spec.html
 */
namespace protocol {

#pragma pack(push, 1)
	/*! @brief Data structure used as the header for FastCGI records!
	 * This structure defines the header used in FastCGI records. It can be casted 
	 * to and from raw 8 byte blocks of data and transmitted/received as is. The
	 * endianess and order of data is kept correct through the accessor member functions.
	 */
	class Header {
	public:
		/*! @brief Set the version field of the record header!
		 * @param[in] version FastCgI protocol version number
		 */
		void set_version(uint8_t version) { this->version = version; }
		/*! @brief Get the version field of the record header!
		 * @return version FastCgI protocol version number
		 */
		int get_version() const { return version; }

		/*! @brief Set the record type in the header!
		 * @param[in] type Record type
		 */
		void set_type(Record_type type) {
			this->type = static_cast<uint8_t>(type);
		}

		/*! @brief Get the record type in the header!
		 * @return Record type
		 */
		Record_type get_type() const {
			return static_cast<Record_type>(type);
		}
		/*! @brief Set the request ID field in the record header!
		 * @param[in] request_id The records request ID
		 */
		void set_request_id(Request_id request_id) {
			this->request_id.value = ntohs(request_id);
		}
		/*! @brief Get the request ID field in the record header!
		 * @return The records request ID
		 */

		Request_id get_request_id() const {
			return ntohs(request_id.value);
		}

		/*! @brief Set the content length field in the record header!
		 * @param[in] content_length The records content length
		 */
		void set_content_length(uint16_t content_length) {
			this->content_length.value = ntohs(content_length);
		}
		/*! @brief Get the content length field in the record header!
		 * @return The records content length
		 */
		uint16_t get_content_length() const {
			return ntohs(content_length.value);
		}
		/*! @brief Set the padding length field in the record header!
		 * @param[in] padding_length The records padding length
		 */
		void set_padding_length(uint8_t padding_length) {
			this->padding_length = padding_length;
		}
		/*! @brief Get the padding length field in the record header!
		 * @return The records padding length
		 */
		uint8_t get_padding_length() const {
			return padding_length;
		}
	private:
		//! FastCGI version number
		uint8_t version;
		//! Record type
		uint8_t type;
		u16_t request_id;
		u16_t content_length;
		//! Length of record padding
		uint8_t padding_length;
		//! Reseved for future use and header padding
		uint8_t reserved;
	};
#pragma pack(pop)

}

MOSH_FCGI_END

#endif
