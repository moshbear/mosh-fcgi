//! @file  mosh/fcgi/bits/utf8.hpp UTF-8 encoding/decoding interface
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

#ifndef MOSH_FCGI_UTF8_HPP
#define MOSH_FCGI_UTF8_HPP

#include <cstddef>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @brief Decode UTF-8 data to UTF-16/32 (depending on sizeof(wchar_t))
 *
 * Decodes bytes from [from, from_end) and places them into the buffer
 * 	[to, to_end).
 *
 * A pointer to the first untouched input is stored in from_next and a
 * pointer to the fist untouched output is stored in to_next.
 *
 * @retval -errno Conversion error
 * @retval 0
 * @retval 1 Partial decoding - output buffer denoted by [to, to_end) too small
 */
int utf8_in(const uchar* from, const uchar* from_end, const uchar *& from_next,
		wchar_t* to, wchar_t* to_end, wchar_t*& to_next);
/*! @brief Encode UTF-16/32 (depending on sizeof(wchar_t)) to UTF-8
 *
 * Decodes bytes from [from, from_end) and places them into the buffer
 * 	[to, to_end).
 *
 * A pointer to the first untouched input is stored in from_next and a
 * pointer to the fist untouched output is stored in to_next.
 *
 * @retval -errno Conversion error
 * @retval 0
 * @retval 1 Partial decoding - output buffer denoted by [to, to_end) too small
 */
int utf8_out(const wchar_t* from, const wchar_t* from_end, const wchar_t *& from_next,
		uchar* to, uchar* to_end, uchar *& to_next);
//! Get the number of code points represented by the UTF-8 in [from, from_end)
std::size_t utf8_length(const uchar* from, const uchar* from_end, std::size_t limit);

#endif

