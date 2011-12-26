//! @file protocol/vars.hpp Protocol constants and pseudo-constants
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


#ifndef MOSH_FCGI_PROTOCOL_VARS_HPP
#define MOSH_FCGI_PROTOCOL_VARS_HPP

#include <map>
#include <string>
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
	
	//! Defines text labels for the Record_type values
	extern const char* record_type_labels[];
	
	//! The version of the FastCGI protocol that this adheres to
	const uint8_t version = 1;
	//! All FastCGI records will be a multiple of this many bytes
	const size_t chunk_size = 8;

	//! Reply record that will be sent when asked the maximum allowed file descriptors open at a time
	extern Management_reply<14, 2, 8> max_conns_reply;
	//! Reply record that will be sent when asked the maximum allowed requests at a time
	extern Management_reply<13, 2, 1> max_reqs_reply;
	//! Reply record that will be sent when asked if requests can be multiplexed over a single connections
	extern Management_reply<15, 1, 8> mpxs_conns_reply;
}

MOSH_FCGI_END

#endif

