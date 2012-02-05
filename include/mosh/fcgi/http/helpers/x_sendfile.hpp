//! @file  mosh/fcgi/http/helpers/x_sendfile.hpp Header for X-SendFile
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

#ifndef MOSH_FCGI_HTTP_HELPERS_XSENDFILE_HPP
#define MOSH_FCGI_HTTP_HELPERS_XSENDFILE_HPP

#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/http/helpers/helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

//! Header helpers
namespace helpers {

//! HTTP X-Sendfile headers
namespace xsendfile {

// Note: is_nginx is unsigned instead of bool so that function signatures for (string,numeric) match

/*! @brief Generate a X-Sendfile header
 *  @param[in] loc Location
 *  @param[in] nginx nginx mode (X-Accel-Redirect)
 *  @return { !nginx ? "X-SendFile" : "X-Accel-Redirect", loc }
 */
Helper::header_pair print_xsendfile(const ustring& loc, unsigned is_nginx = 0);

//! Create a helper consisting of redirection generators
Helper helper();

}

}

}

MOSH_FCGI_END

#endif

