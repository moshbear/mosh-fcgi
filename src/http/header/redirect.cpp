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

#include <memory>
#include <stdexcept>
#include <string> 
#include <mosh/fcgi/http/header/helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/http/header_default.hpp>
#include <src/namespace.hpp>

namespace {

SRC::http::header::Helper_smartptr status;

struct redir : public virtual MOSH_FCGI::http::header::Helper {
	Helper::header_pair operator()(unsigned code, std::string const& loc) {
		if ((code / 100) != 3) {
			throw std::invalid_argument("http_code must be 3xx (redirection-related)");
		}
		if (!status)
			status = SRC::http::header::status();
		Helper::header_pair _st = (*status)(code);
		_st.push_back({"Location", loc});
		return _st;
	}
};

}

SRC_BEGIN

namespace http { namespace header {

Helper_smartptr redirect() {
	return Helper_smartptr(new redir);
}

} }

SRC_END

