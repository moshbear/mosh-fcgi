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


#ifndef MOSH_FCGI_END_REQUEST_HPP
#define MOSH_FCGI_END_REQUEST_HPP

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

		//! Requests Status

	/*! @brief Data structure used as the body for FastCGI records with a RecordType of end_request
	 * This structure defines the body used in FastCGI end_request records. It can be casted 
	 * to raw 8 byte blocks of data and transmitted as is. An end_request record is sent when
	 * this side wishes to terminate a request. This can be simply because it is complete or
	 * because of a problem.
	 */
	class End_request {
	public:
		/*! @brief Set the requests return value!
		 * This is an integer value representing what would otherwise be the return value in a
		 * normal CGI application.
		 *
		 * @param[in] status The return value
		 */
		void set_app_status(int status) { app_status.value =  ntohl(status); }
		/*! @brief Set the reason for termination!
		 * This value is one of Protocol_status and represents the reason for termination.
		 *
		 * @param[in] status The requests status
		 */
		void set_protocol_status(Protocol_status status) { protocol_status = static_cast<uint8_t>(status); }
	private:
		union {
			struct {
				uint8_t msb;
				uint8_t msb2;
				uint8_t lsb1;
				uint8_t lsb;
			};
			uint32_t value;
		} app_status;
		//! Requests Status
		uint8_t protocol_status;
		//! Reseved for future use and body padding
		uint8_t reserved[3];
	};
}

#pragma pack(pop)
MOSH_FCGI_END

#endif
