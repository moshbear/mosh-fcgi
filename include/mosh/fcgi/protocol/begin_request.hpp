//! @file protocol/begin_request.hpp FastCGI BEGIN_REQUEST record
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


#ifndef MOSH_FCGI_PROTOCOL_BEGIN_REQUEST_HPP
#define MOSH_FCGI_PROTOCOL_BEGIN_REQUEST_HPP

#include <cstdint>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>


MOSH_FCGI_BEGIN

namespace protocol {

#pragma pack(push, 1)
		
	/*! @brief Data structure used as the body for FastCGI records with a RecordType of %begin_request
	 *
	 * This structure defines the body used in FastCGI Begin_request records. It can be casted 
	 * from raw 8 byte blocks of data and received as is. A Begin_request record is received
	 * when the other side wished to make a new request.
	 */
	class Begin_request {
	public:
		/*! @brief Get the role field from the record body
		 *  @return The expected Role that the request will play
		 */
		_g_role role() const { return _g_role(_role); }
		/*! @brief Get keep alive value from the record body
		 *
		 * If this value is false, the socket should be closed on our side when the request is complete.
		 * If true, the other side will close the socket when done and potentially reuse the socket and
		 * multiplex other requests on it.
		 *
		 * @return Boolean value as to whether or not the connection is kept alive
		 */
		bool keep_conn() const {
			return _flags & _do_keep_conn;
		}
	private:
		//! Flag bit representing the keep alive value
		static const uint8_t _do_keep_conn = 1;
		uint16_t _role;
		//! Flag value
		uint8_t _flags;
		//! Reseved for future use and body padding
		uint8_t _reserved[5];
	};
#pragma pack(pop)
}

MOSH_FCGI_END

#endif
