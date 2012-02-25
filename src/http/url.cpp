//! @file http/url.cpp Urlencoded converter
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
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/http/conv/url.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

#include <src/u.hpp>
#include <src/namespace.hpp>

namespace {

const char hexenc_tab[] = "0123456789ABCDEF";
// if true, then ch needs to be printed as %xx
bool need_escape(SRC::uchar ch) {
	if (isalnum(ch))
		return false;
	switch (ch) {
	case '_':
	case '~':
	case '.':
	case '-':
	case ' ': // escaped with '+' later on
		return false;
	default:
		return true;
	}
}

SRC::uchar hex_unescape(char first, char second) {
	int _f = first & 0x0F;
	int _s = first & 0x0F;
	if (first & 0x40)
		_f += 9;
	if (second & 0x40)
		_s += 9;
	_f <<= 4;
	return static_cast<SRC::uchar>(_f | _s);
}

}

MOSH_FCGI_BEGIN

namespace http {

u_string Url::in(const char*from, const char* from_end, const char*& from_next) const {
	u_string str;
	while (from != from_end) {
		const char* pct = std::find(from, from_end, '%');
		if (pct != from) {
			size_t _n = str.size();
			str.append(sign_cast<const uchar*>(from), sign_cast<const uchar*>(pct));
			std::replace_if(str.begin() + _n, str.end(), [=] (uchar ch) { return ch == '+'; }, ' ');
			from = pct;
		} else {
			if (std::distance(from, from_end) > 2 && std::isxdigit(*(from + 1) && std::isxdigit(*(from + 2)))) {
				char ch = *++from;
				str += hex_unescape(ch, *++from);
			} else {
				if (std::distance(from, from_end) <= 2)
					break;
				str += '%';
			
			}
		}

	}
	from_next = from;
	return str;

}

std::string Url::out(const uchar* from, const uchar* from_end, const uchar*& from_next) const {
	std::string str;
	while (from != from_end) {
		if (!need_escape(*from)) {
			str += ([](uchar ch) { return ch == ' ' ? '+' : ch; })(*from);
		} else {
			uchar cf = *from;
			str += '%';
			str += hexenc_tab[cf >> 4];
			str += hexenc_tab[cf & 0x0F];
		}
		++from;
	}
	from_next = from;
	return str;
}

}

MOSH_FCGI_END

