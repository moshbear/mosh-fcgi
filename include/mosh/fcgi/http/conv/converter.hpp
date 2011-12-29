//! @file  mosh/fcgi/http/conv/converter.hpp Converter base class
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

#ifndef MOSH_FCGI_HTTP_CONV_CONVERTER_HPP
#define MOSH_FCGI_HTTP_CONV_CONVERTER_HPP

#include <string>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

//! Base class for %Content-Transfer-Encoding converters.
struct Converter {
	/*! @brief Decode a string.
	 *
	 * @param[in] in Start of encoded data
	 * @param[in] in_end End of encoded data
	 * @param[in] in_next On return, this points to the first untranslated character
	 * @return decoded input
	 */
	virtual std::string in(const char* in, const char* in_end, const char*& in_next) const = 0;
	/*! @brief Encode a string.
	 *
	 * @param[in] in Start of decoded data
	 * @param[in] in_end End of decoded data
	 * @param[in] in_next On return, this points to the first untranslated character
	 * @return encoded input
	 */
	virtual std::string out(const char* in, const char* in_end, const char*& in_next) const = 0;
	
};

/*! @brief Get the converter that handles a specified encoding
 *
 * Uses @c new for allocation.
 * @param[in] c_name encoding name
 * @return pointer to @c Converter instance that handles the encoding
 */
Converter* get_conv(const std::string& c_name);

}

MOSH_FCGI_END

#endif
