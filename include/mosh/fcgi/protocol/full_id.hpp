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


#ifndef MOSH_FCGI_PROTOCOL_FULL_ID_HPP
#define MOSH_FCGI_PROTOCOL_FULL_ID_HPP

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
	/*! @brief  A full ID value for a FasTCGI request!
	 * Because each FastCGI request has a RequestID and a file descriptor
	 * associated with it, this class defines an ID value that encompasses
	 * both. The file descriptor is stored internally as a 16 bit unsigned
	 * integer in order to keep the data structures size at 32 bits for
	 * optimized indexing.
	 */
	struct Full_id {
		/*! @brief  Construct from a FastCGI RequestID and a file descriptor!
		 * The constructor builds upon a RequestID and the file descriptor
		 * it is communicating through.
		 *
		 * @param  [in] fcgi_id The FastCGI request ID
		 * @param [in] fd The file descriptor
		 */
		Full_id(Request_id fcgi_id, int fd)
		: fcgi_id(fcgi_id), fd(fd)
		{ } 

		Full_id(uint32_t f)
		: full(f)
		{ }

		Full_id() { }
		union {
			struct {
				//! FastCGI Request ID
				Request_id fcgi_id;
				//! Associated File Descriptor
				uint16_t fd;
			};
			uint32_t full;
		};
		//!Compare between two Full_id variables
		bool operator > (const Full_id& x) const {
			return full > x.full;
		}
		bool operator < (const Full_id& x) const {
			return full < x.full;
		}
		//!Compare between two Full_id variables
		bool operator == (const Full_id& x) const {
			return full == x.full;
		}
	};
	
}

MOSH_FCGI_END

#endif
