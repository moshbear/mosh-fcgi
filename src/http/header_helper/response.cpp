//! @file http/header_helper/response.cpp - HTTP response
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
#include <mosh/fcgi/http/helpers/response.hpp>
#include <mosh/fcgi/http/helpers/status_helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

namespace helpers {

//! HTTP response headers
namespace response {

/*! @brief Generate a HTTP response header
 *  @param[in] ver HTTP version
 *  @param[in] code HTTP status code
 *  @return HTTP/$ver $code $(status($code))
 *  @sa status_helper
 */
Helper::header_pair print_response(const std::string& ver, unsigned code) {
	std::basic_stringstream ss;
	ss << "HTTP/" << ver;
	ss << ' ' << code;
	ss << ' ' << status_helper::get_string(code);
	return { std::make_pair("@@http_response@@", ss.str()) };
}

//! Create a helper consisting of response generators
Helper helper() {
	Helper h;
	h.do_s_u = print_response;
	return h;
}

} // response

} // helpers

} // http

MOSH_FCGI_END

