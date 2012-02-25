//! @file http/header_helpers/status.cpp - HTTP status codes
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

#include <sstream>
#include <utility>
#include <string>
#include <mosh/fcgi/http/helpers/helper.hpp>
#include <mosh/fcgi/http/helpers/status_helper.hpp>
#include <mosh/fcgi/http/helpers/status.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

namespace helpers {

//! HTTP status headers
namespace status {
	
/*! @brief Generate a HTTP @c status header
 *  @param[in] st HTTP status code
 *  @return status header with corresponding code
 *  @sa status_helper
 */
Helper::header_pair print_status (unsigned st) {
	std::stringstream ss;
	ss << st;
	ss << " " + status_helper::get_string(st);
	return { std::make_pair("Status", ss.str()) };
}
	
//! Create a helper consisting of status line generators 
Helper helper() {
	Helper h;
	h.do_u = print_status;
	return h;
}

} // status

} // helpers

} // http

MOSH_FCGI_END


