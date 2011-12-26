//! @file mosh/fcgi/bits/ci_strcomp.hpp Case insensitive string comparison
/*
 *  Copyright (C) 1996 - 2007 GNU Cgicc team
 *                2011 m0shbear
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

#ifndef MOSH_FCGI_CI_STRCOMP_HPP
#define MOSH_FCGI_CI_STRCOMP_HPP

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <string>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

template <typename char_type> char_type _xlower(const char_type&);
template<> char _xlower<char>(const char& ch) { return std::tolower(ch); }
template<> wchar_t _xlower<wchar_t>(const wchar_t& wc) { return std::towlower(wc); }

/*!
 * @brief Compare two strings for equality, ignoring case.
 *
 * For case-sensitive comparison, use (s1 == s2);
 * @tparam charT (deduced from input)
 * @param s1 The first string to compare
 * @param s2 The second string to compare
 * @return @c true if the strings are equal, @c false if they are not
 */
template <class charT>
bool ci_equality(const std::basic_string<charT>& s1, const std::basic_string<charT>& s2)
{
	return std::equal(s1.begin(), s1.end(), s2.begin(),
			[](const charT& c1, const charT& c2) { return _xlower(c1) == _xlower(c2); });
}
/*!
 * @brief Compare two strings for equality, ignoring case.
 *
 * For case-sensitive comparison, use (s1 == s2);
 * @tparam charT (deduced from input)
 * @param s1 The first string to compare
 * @param s2 The second string to compare
 * @param n The number of characters to compare.
 * @return @c true if the strings are equal, @c false if they are not
 */
template <class charT>
bool ci_equality(const std::basic_string<charT>& ss1, const std::basic_string<charT>& ss2, size_t n)
{

	return std::equal(ss1.begin(), ss1.begin() + n, ss2.begin(),
				[](const charT& c1, const charT& c2) { return _xlower(c1) == _xlower(c2); });
}


MOSH_FCGI_END

#endif
