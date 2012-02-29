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

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <mosh/fcgi/http/header/helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/http/header_default.hpp>
#include <src/namespace.hpp>

namespace {

std::string xsf_string(unsigned i) {
	switch (i) {
	case 0: return "X-Sendfile"; // normal
	case 1: return "X-Accel-Redirect"; // nginx
	default:;
	}
	throw std::invalid_argument("xsf string not found");
}

struct xsf : public virtual MOSH_FCGI::http::header::Helper {
	Helper::header_pair operator()(std::string const& loc) {
		return (*this)(loc, 0);
	}
	Helper::header_pair operator()(std::string const& loc, unsigned compat) {
		return { std::make_pair(xsf_string(compat), loc) };
	}
};

}
	
SRC_BEGIN

namespace http { namespace header {

Helper_smartptr x_sendfile() {
	return Helper_smartptr(new xsf);
}

} }

SRC_END

