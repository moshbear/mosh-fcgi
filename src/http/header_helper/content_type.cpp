//! @file http/header_helper/content_type.cpp - Content-Type
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
#include <utility>
#include <mosh/fcgi/http/helpers/helper.hpp>
#include <mosh/fcgi/http/helpers/content_type.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

namespace helpers {

//! Content-Type headers
namespace content_type {

Helper::header_pair print_ct(const std::string& ctype) {
	return { std::make_pair("Content-Type: ", ctype) };
}

Helper::header_pair print_ct_with_cs(const std::string& ctype, const std::string& cset) {
	if (cset.empty())
		return print_ct(ctype);
	return { std::make_pair("Content-Type", ctype + "; charset=" + cset + "\r\n") };
}

Helper helper() {
	Helper h;
	h.do_s = print_ct;
	h.do_s_s = print_ct_with_cs;
	return h;
}

} // content_type

} // helpers

} // http

MOSH_FCGI_END

