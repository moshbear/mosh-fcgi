//! @file http/converter.cpp - Base converter class
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*                                                                          *
* This file is part of mosh-fcgi.                                          *
*                                                                          *
* mosh-fcgi is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* mosh-fcgi is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with mosh-fcgi.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/

#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/http/conv.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

/*! @brief Get the converter that handles a specified encoding
 * Uses @c new for allocation.
 * @param[in] c_name encoding name
 * @return pointer to @c Converter instance that handles the encoding
 */
Converter* get_conv(const u_string& c_name) {
	if (c_name == "quoted-printable")
		return new Qp;
	if (c_name == "base64")
		return new Base64;
	if (c_name == "url-encoded")
		return new Url;
	return 0;
}

}

MOSH_FCGI_END

