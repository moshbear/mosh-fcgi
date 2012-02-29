//! @file  mosh/fcgi/http/header/helper.hpp HTTP Header helper factory
/*
 * Copyright (C) 2012 m0shbear
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA 
 */

#ifndef MOSH_FCGI_HTTP_HEADER_HELPER_HPP
#define MOSH_FCGI_HTTP_HEADER_HELPER_HPP

#include <memory>
#include <string>
#include <vector>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

namespace header {

//! HTTP header generator factory
struct Helper {

	typedef std::vector<std::pair<std::string, std::string>> header_pair;
	// can't PV here because allocation will fail otherwise
	virtual header_pair operator()(unsigned);
	virtual header_pair operator()(std::string const&);
	virtual header_pair operator()(unsigned, std::string const&);
	virtual header_pair operator()(unsigned, unsigned);
	virtual header_pair operator()(std::string const&, unsigned);
	virtual header_pair operator()(std::string const&, std::string const&);

	virtual ~Helper() { }
};

}

}
	
MOSH_FCGI_END

#endif
