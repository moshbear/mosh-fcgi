//! @file  mosh/fcgi/bits/u.hpp - u_* stuff
/***************************************************************************
* Copyright (C) 2012 m0shbear                                              *
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

#ifndef MOSH_FCGI_U_HPP
#define MOSH_FCGI_U_HPP

#include <type_traits>
#include <string>
#include <cstring>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! Typedef for byte
typedef unsigned char uchar;
//! Typedef for a string of unsigned chars
typedef std::basic_string<uchar> u_string;

/*! @name sign_cast 
 *  @brief Adds or removes sign attributes from a class
 *
 *  Similar to how const_cast adds or removes cv-qualification from a class, this function adds or removes
 *  sign attributes from a class.
 *
 *  @note This function is restricted to pointer and reference types, and will not convert between
 *  @note types that have differing cv-qualifiers.
 *  @note
 *  @note Unless @c MOSH_FCGI_SIGN_CAST_NO_CHAR_RESTRICT is defined, this function will only accept
 *  @note pointers and references to ((un?signed)? char.
 *
 *  @tparam To destination type
 *  @tparam From source type ( inferred from @c expr )
 *  @param expr Expression
 *  @{
 */
//! Adds or removes sign attributes from a a class (pointer version)
template <typename To, typename From>
typename
std::enable_if<(
		// Check that both To and From are pointers
		std::is_pointer<To>::value && std::is_pointer<From>::value
			// Assert that both To and From have the same sign-punned type
			&& std::is_same<typename std::make_unsigned<typename std::remove_pointer<To>::type>::type, 
					typename std::make_unsigned<typename std::remove_pointer<From>::type>::type
				>::value
#ifndef MOSH_FCGI_SIGN_CAST_NO_CHAR_RESTRICT
			// Check that both To and From are variants of char
			// We only need to check To because To == From, as per the previous assertion
			&& std::is_same<uchar, typename std::remove_cv<typename std::make_unsigned<
						typename std::remove_pointer<To>::type
						>::type>::type
					>::value
#endif
	),
	To
>::type sign_cast(From expr) {
	return reinterpret_cast<To>(expr);
}
#if 0
//! Adds or removes sign attributes from a class (reference version)
template <typename To, typename From>
typename
std::enable_if<(
		// Check that both To and From are references
		std::is_reference<To>::value && std::is_reference<From>::value
		&& !std::is_pointer<typename std::remove_reference<To>::type>::value && !std::is_pointer<typename std::remove_reference<From>::type>::value
			// Assert that both To and From have the same sign-punned type
			&& std::is_same<typename std::make_unsigned<typename std::remove_reference<To>::type>::type, 
					typename std::make_unsigned<typename std::remove_reference<From>::type>::type
				>::value
#ifndef MOSH_FCGI_SIGN_CAST_NO_CHAR_RESTRICT
			// Check that both To and From are variants of char
			// We only need to check To because To == From, as per the previous assertion
			&& std::is_same<uchar, typename std::remove_cv<typename std::make_unsigned<
						typename std::remove_reference<To>::type
						>::type>::type
					>::value
#endif
	),
	To
>::type sign_cast(From expr) {
	return reinterpret_cast<To>(expr);
}
#endif
//@}			

MOSH_FCGI_END

#endif
