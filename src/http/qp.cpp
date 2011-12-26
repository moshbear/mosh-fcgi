//! @file http/qp.cpp @c Quoted-printable converter
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
#include <string>
#include <cctype>
#include <cstddef>

#include <mosh/fcgi/http/conv/qp.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {

const char hexenc_tab[] = "0123456789ABCDEF";
// if true, then ch needs to be printed as =xx
bool need_escape(char ch) {
	if (ch >= 0x80)
		return true;
	if (isalnum(ch))
		return false;
	if (ch == '=')
		return true;
	if (ispunct(ch))
		return false;
	return true;
}

unsigned char hex_unescape(char first, char second) {
	int _f = first & 0x0F;
	int _s = first & 0x0F;
	if (first & 0x40)
		_f += 9;
	if (second & 0x40)
		_s += 9;
	_f <<= 4;
	return static_cast<unsigned char>(_f | _s);
}

}

MOSH_FCGI_BEGIN

namespace http {

std::string Qp::in(const char* from, const char* from_end, const char *& from_next) const {
	std::string str;
	while (from != from_end) {
		const char* eq = std::find(from, from_end, '=');
		if (eq != from) {
			str.append(from, eq);
			from = eq;
		} else {
			if (std::distance(from, from_end) > 2
			&& ((std::isxdigit(*(from + 1))
					&& std::isxdigit(*(from + 2))
				)
				||(*(from+1) == 0x0D && *(from + 2) == 0x0A)
			)) {
				char ch = *++from;
				if (ch == 0x0D) { // NBSP
					++from;
				} else {
					str += hex_unescape(ch, *++from);
				}
			} else {
				if (std::distance(from, from_end) <= 2)
					break;
				str += '=';
			
			}
		}

	}
	from_next = from;
	return str;	
}

std::string Qp::out(const char* from, const char* from_end, const char*& from_next) const {
	std::string str;
	while (from != from_end) {
		if (!need_escape(*from)) {
			str += *from;
		} else {
			unsigned char cf = *from;
			str += '=';
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

