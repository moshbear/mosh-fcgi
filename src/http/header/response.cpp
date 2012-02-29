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

#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <mosh/fcgi/http/header/helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/http/header_default.hpp>
#include <src/namespace.hpp>

namespace {

SRC::http::header::Helper_smartptr status;

struct response : public virtual MOSH_FCGI::http::header::Helper {
	Helper::header_pair operator()(unsigned ver, unsigned code) {
		if (!status)
			status = SRC::http::header::status();
		std::stringstream ss;
		ss << "HTTP/" << ((ver & 0xFF00) >> 8) << '.' << (ver & 0x00FF);
		ss << ' ' << (*status)(code)[0].second;
		return { std::make_pair("@@http_response@@", ss.str()) };
	}
};

}


SRC_BEGIN

namespace http { namespace header {

Helper_smartptr response() {
	return Helper_smartptr(new ::response);
}

} }

SRC_END

