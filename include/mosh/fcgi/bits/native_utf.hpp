//! @file  mosh/fcgi/bits/native_utf.hpp Native UTF type
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

#ifndef MOSH_FCGI_NATIVE_UTF_HPP
#define MOSH_FCGI_NATIVE_UTF_HPP

#include <string>
#include <limits>
extern "C" {
#include <netinet/in.h>
#include <arpa/inet.h>
}

#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @brief Get the platform's native UTF
 *
 * Depending on the value of N, it returns the native (endian-wise) encoding
 * for UTF-*.
 *
 * When N = 1, it returns either ASCII or ISO-8859-1, depending on the signedness of char.
 *
 * When N = 2, it returns UTF-16, with the proper endianness.
 *
 * When N = 4, it returns UTF-32, with the proper endianness.
 *
 * @tparam N native UTF character size
 */
template <size_t N> 
struct native_utf {
	static std::string value();
};

template<> std::string native_utf<1>::value() {
	if (std::numeric_limits<char>::is_signed)
		return "US-ASCII"; // char is signed, must go 7-bit
	else
		return "ISO-8859-1";
}

template<> std::string native_utf<2>::value() {
	if (htons(1) == ntohs(1))
		return "UTF-16BE";
	else
		return "UTF-16LE";
}

template<> std::string native_utf<4>::value() {
	if (htonl(1) == ntohl(1))
		return "UTF32BE";
	else
		return "UTF32LE";
}

MOSH_FCGI_END

#endif

