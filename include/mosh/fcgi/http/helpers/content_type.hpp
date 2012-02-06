//! @file  mosh/fcgi/http/helpers/content_type.hpp Content-Type header
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

#ifndef MOSH_FCGI_HTTP_HELPERS_CONTENT_TYPE_HPP
#define MOSH_FCGI_HTTP_HELPERS_CONTENT_TYPE_HPP

#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/http/helpers/helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

//! Header helpers
namespace helpers {

//! Content-Type headers
namespace content_type {

/*! @brief Generate Content-Type header
 *  @param[in] ctype content type
 *  @return { { "Content-Type", "<ctype>" } }
 */
Helper::header_pair print_ct(const ustring& ctype);

/*! @brief Generate Content-Type header
 *  @param[in] ctype content type
 *  @param[in] cset charset
 *  @return { { "Content-Type", "<ctype>; charset=<cset>" } }
 */
Helper::header_pair print_ct_with_cs(const ustring& ctype, const ustring& cset);
	
//! Create a helper consisting of Content-Type generators
Helper helper();

}

}

}

MOSH_FCGI_END

#endif

