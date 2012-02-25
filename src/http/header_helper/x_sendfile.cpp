//! @file  http/header_helper/x_sendfile.cpp - X-Sendfile
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
#include <mosh/fcgi/http/helpers/x_sendfile.hpp>
#include <mosh/fcgi/http/helpers/helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {
	MOSH_FCGI::http::Helper::header_pair print_xsf(const std::string& loc) {
		return MOSH_FCGI::http::helpers::xsendfile::print_xsendfile(loc, 0);
	}
}

MOSH_FCGI_BEGIN

namespace http {

//! Header helpers
namespace helpers {

//! HTTP X-Sendfile headers
namespace xsendfile {

// Note: is_nginx is unsigned instead of bool so that function signatures for (string,numeric) match
Helper::header_pair print_xsendfile(const std::string& loc, unsigned is_nginx) {
	return { std::make_pair(is_nginx ? "X-Accel-Redirect" : "X-Sendfile", loc) };
}

//! Create a helper consisting of X-Sendfile generators
Helper helper() {
	Helper h;
	h.do_s_u = print_xsendfile;
	h.do_s = print_xsf;
}

}

}

}

MOSH_FCGI_END

