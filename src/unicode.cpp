//! @file unicode.cpp - utf8 encode/decode
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

#include <locale>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstdint>

#include <mosh/fcgi/unicode.hpp>
#include <mosh/fcgi/bits/utf8_cvt.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

template<> std::wstring unicode::in<wchar_t>(const char* in, const char* in_end, const char*& in_next) {
	using namespace std;
	wstring ret;
	ret.resize(in_end - in);
	mbstate_t dummy;
	wchar_t* r_next;
	use_facet<codecvt<wchar_t, char, mbstate_t> >(locale(locale::classic(), new Utf8_cvt))
		.in(dummy, in, in_end, in_next, &(*ret.begin()), &(*ret.end()), r_next);
	ret.resize(r_next - ret.data());
	return ret;
}

template<> std::string unicode::out<wchar_t>(const wchar_t* in, const wchar_t* in_end, const wchar_t*& in_next) {
	using namespace std;
	string ret;
	ret.resize(in_end - in);
	mbstate_t dummy;
	char* r_next;
	use_facet<codecvt<wchar_t, char, mbstate_t> >(locale(locale::classic(), new Utf8_cvt))
		.out(dummy, in, in_end, in_next, &(*ret.begin()), &(*ret.end()), r_next);
	ret.resize(r_next - ret.data());
	return ret;
}

MOSH_FCGI_END
