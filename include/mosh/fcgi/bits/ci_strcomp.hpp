//! @file  mosh/fcgi/bits/ci_strcomp.hpp Case insensitive string comparison
/*
 *  Copyright (C) 2011-2 m0shbear
 *                1996 - 2007 GNU Cgicc team
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
#include <mosh/fcgi/bits/cmp.hpp>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

template <typename char_type> char_type _xlower(const char_type&);
template<> char _xlower<char>(const char& ch) { return std::tolower(ch); }
template<> uchar _xlower<uchar>(const uchar& ch) { return std::tolower(ch); }
template<> wchar_t _xlower<wchar_t>(const wchar_t& wc) { return std::towlower(wc); }

// Case-insensitive char comparator
template <class charT>
bool ci_charcmp(charT&& ch1, charT&& ch2, Cmp_test test) {
	switch (test) {
	case Cmp_test::eq: return (_xlower(o1) == _xlower(o2));
	case Cmp_test::ne: return (_xlower(o1) != _xlower(o2));
	case Cmp_test::lt: return (_xlower(o1) < _xlower(o2));
	case Cmp_test::le: return (_xlower(o1) <= _xlower(o2));
	case Cmp_test::ge: return (_xlower(o1) >= _xlower(o2));
	case Cmp_test::gt: return (_xlower(o1) > _xlower(o2));
	}
	return false;
}

/*!
 * @brief Compare two strings for a condition, ignoring case.
 *
 * For case-sensitive comparison, use cmp(s1, s2, op).
 * @tparam charT (deduced from input)
 * @param s1 The first string to compare
 * @param s2 The second string to compare
 * @param op The comparison mode to use
 * @return @c true if the comparison succeeds
 */
template <class charT>
bool ci_cmp(std::basic_string<charT>&& s1, std::basic_string<charT>&& s2, Cmp_test op = Cmp_test::eq)
{
	return range_cmp(s1, s2, op,
				[] (charT&& ch1, charT&& ch2) {
					return ci_charcmp<charT>(ch1, ch2, op);
				}
			);
}

/*!
 * @brief Compare two strings for a condition, ignoring case.
 *
 * For case-sensitive comparison, use range_cmp(s1, s2, n, op).
 * @tparam charT (deduced from input)
 * @param ss1 The first string to compare
 * @param ss2 The second string to compare
 * @param n The number of characters to compare.
 * @return @c true if the comparison succeeds
 */
template <class charT>
bool ci_cmp(std::basic_string<charT>&& ss1, std::basic_string<charT>&& ss2, size_t n, Cmp_test op = Cmp_test::eq)
{
	return range_cmp(s1, s2, n, op,
				[] (charT&& ch1, charT&& ch2) {
					return ci_charcmp<charT>(ch1, ch2, op);
				}
			);
}

template <class charT, Cmp_test cmp_t = Cmp_test::eq>
struct ci_cmp_wrapper {
	bool operator()(const std::basic_string<charT>& s1, const std::basic_string<charT>& s2) const {
		return ci_cmp(s1, s2, cmp_t);
	}
};

MOSH_FCGI_END

#endif
