//! @file mosh/fcgi/http/conv/base64.hpp - Base64 converter
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*                                                                          *
* This file is part of fastcgi++.                                          *
*                                                                          *
* fastcgi++ is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/
#ifndef MOSH_FCGI_HTTP_BASE64_HPP
#define MOSH_FCGI_HTTP_BASE64_HPP

#include <string>
#include <stdexcept>
#include <mosh/fcgi/http/conv/converter.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

struct Base64 : public Converter {
	/*! @brief Decode a quoted-printable string.
	 *
	 * @param[in] in Start of encoded data
	 * @param[in] in_end End of encoded data
	 * @param[in] in_next On return, this points to the first untranslated character
	 * @return decoded input
	 */
	std::string in(const char* in, const char* in_end, const char*& in_next) const;
	/*! @brief Encode a quoted-printable string [UNIMPLEMENTED]
	 * \warning This function is not implemented.
	 * \warning Using it will throw a std::logic_errror.
	 * @throws std::logic_error if function call is attempted
	 * @param[in] in Start of decoded data
	 * @param[in] in_end End of decoded data
	 * @param[in] in_next On return, this points to the first untranslated character
	 * @return encoded input
	 */
	std::string out(const char* in, const char* in_end, const char*& in_next) const {
		throw std::logic_error("mosh::fcgi::http::Base64::out: UNIMPLEMENTED");
	}
};

}

MOSH_FCGI_END

#endif
