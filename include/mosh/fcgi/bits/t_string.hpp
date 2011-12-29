//! @file  mosh/fcgi/bits/t_string.hpp String widening macros
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
#ifndef MOSH_FCGI_T_STRING_HPP
#define MOSH_FCGI_T_STRING_HPP

#include <string>
#include <locale>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

template <typename T>
T wide_char(char ch) {
	// It's a bit roundabout, but this is the only way to get the current locale
	std::locale loc = std::locale::global(std::locale::classic());
	std::locale::global(loc);
	return std::use_facet<std::ctype<T>>(loc). widen(ch);
}

template <typename T>
std::basic_string<T> wide_string(const std::string& s) {
	std::basic_string<T> t(s.size(), 0);
	for (size_t i = 0; i < s.size(); ++i)
		t[i] = wide_char<T>(s[i]);
	return t;
}

MOSH_FCGI_END
#endif
