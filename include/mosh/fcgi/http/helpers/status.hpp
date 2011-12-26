//! @file mosh/fcgi/http/helpers/status.hpp String lookup for HTTP status codes
/* 
 *  Copyright (C) 2011 m0shbear
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 */

#ifndef MOSH_FCGI_HTTP_HELPERS_STATUS_HPP
#define MOSH_FCGI_HTTP_HELPERS_STATUS_HPP

#include <string>
#include <mosh/fcgi/http/helpers/helper.hpp>
#include <mosh/fcgi/http/helpers/status_helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

//! Header helpers
namespace helpers {

//! HTTP status headers
namespace status {

/*! @brief Generate a HTTP @c Status header
 *  @param[in] st HTTP status code
 *  @return { { "Status", "<st> <status-string(st)>" } }
 *  @sa status_helper
 */
Helper::header_pair print_status (unsigned st);
	
//! Create a helper consisting of Status line generators 
Helper helper();

}

}

}

MOSH_FCGI_END

#endif

