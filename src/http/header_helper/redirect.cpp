//! @file http/header_helper/redirect.cpp - HTTP redirection
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

#include <string>
#include <sstream>
#include <stdexcept>
#include <mosh/fcgi/http/helpers/helper.hpp>
#include <mosh/fcgi/http/helpers/redirect.hpp>
#include <mosh/fcgi/http/helpers/status.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

namespace helpers {

//! HTTP redirect headers
namespace redirect {
	
Helper::header_pair print_redir(unsigned code, const std::string& loc) {
	if ((code / 100) != 3) {
		throw std::invalid_argument("http_code must be 3xx (redirection-related)");
	}
	Helper::header_pair _st = status::print_status(code);
	_st.push_back({"Location", loc});
	return _st;
}

Helper helper() {
	Helper h;
	h.do_u_s = print_redir;
	return h;
}

} // redirect

} // helpers

} // http

MOSH_FCGI_END

