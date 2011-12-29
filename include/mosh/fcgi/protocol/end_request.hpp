//! @file  mosh/fcgi/protocol/end_request.hpp FastCGI END_REQUEST record
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


#ifndef MOSH_FCGI_END_REQUEST_HPP
#define MOSH_FCGI_END_REQUEST_HPP

#include <cstdint>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/gs.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace protocol {

#pragma pack(push, 1)

	/*! @brief Data structure used as the body for FastCGI records with a RecordType of end_request
	 *
	 * This structure defines the body used in FastCGI end_request records. It can be casted 
	 * to raw 8 byte blocks of data and transmitted as is. An end_request record is sent when
	 * this side wishes to terminate a request. This can be simply because it is complete or
	 * because of a problem.
	 */
	class End_request {
	public:
		End_request() { }
		End_request(uint32_t _app_status_, Protocol_status _proto_status_) {
			app_status() = _app_status_;
			protocol_status() = _proto_status_;
		}
		virtual ~End_request() { }

		//! Get a setter for the request's return value
		_s_u32 app_status() { return _s_u32(_app_status); }
		//! Get a setter for the reason for termination
		_s_status protocol_status() { return _s_status(_protocol_status); }
	private:
		//! Request's exit status
		uint32_t _app_status;
		//! Protocol's termination status
		uint8_t _protocol_status;
		//! Reseved for future use and body padding
		uint8_t reserved[3];
	};
}

#pragma pack(pop)
MOSH_FCGI_END

#endif
