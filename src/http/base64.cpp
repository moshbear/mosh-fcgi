//! @file http/base64.cpp Base64 encode/decode
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
#include <stdexcept>
#include <cstdint>
#include <string>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/http/conv/base64.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

#include <src/u.hpp>
#include <src/namespace.hpp>

namespace {
	
#define UALPHA 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', \
	       'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', \
	       'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', \
	       'Y', 'Z'
#define LALPHA 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', \
	       'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', \
	       'q', 'r', 's', 't', 'u', 'v', 'w', 'x', \
	       'y', 'z'
#define DIGIT '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'

#define CH62 '+'
#define CH63 '/'
#define PAD '='

#define IS_UALPHA(x) (('A' <= (x)) && ((x) <= 'Z'))
#define IS_LALPHA(x) (('a' <= (x)) && ((x) <= 'z'))
#define IS_DIGIT(x) (('0' <= (x)) && ((x) <= '9'))

int8_t b64_val(char ch) {
	if (IS_UALPHA(ch))
		return (ch - 'A');
	if (IS_LALPHA(ch))
		return 26 + (ch - 'a');
	if (IS_DIGIT(ch))
		return 36 + (ch - '0');
	
	switch (ch) {
	case CH62:  return 62;
	case CH63:  return 63;
	case PAD:   return -1;
	default:    return -63;
	}
}


const char b64_tab[64] = { UALPHA, LALPHA, DIGIT, CH62, CH63 };

// Decode the smallest complete _un_padded base64 chunk
// The smallest bytesize possible is gcd(6,8) = 24
// in_bytes needed = 24/6 = 4
// out_bytes needed = 24/8 = 3
int do_in_i4o3(const char* in /* 4 */, SRC::uchar* out /* 3 */) {
	uint32_t chars;
	int eq_cnt = 0;
	for (size_t i = 0; i < 4; ++i) {
		chars <<= 6;
		int k = b64_val(in[i]);
		if (k < -1)
			return -1;
		if (k == -1) {
			k = 0;
			++eq_cnt;
			if (eq_cnt > 2)
				return -eq_cnt;
		}
		chars |= k;
	}
	for (int j0 = 0; j0 < eq_cnt; ++j0)
		chars >>= 8;
	for (size_t j = 2 - eq_cnt; j >= 0; --j) {
		out[j] = static_cast<SRC::uchar>(chars & 0xFF);
		chars >>= 8;
	}
	return (3 - eq_cnt);
}

// Encode the smallest complete _un_padded base64 chunk
// The smallest bytesize possible is gcd(6,8) = 24
// in_bytes needed = 24/8 = 3
// out_bytes needed = 24/6 = 4
int do_out_i3o4(const SRC::uchar* in /* 3 */, char* out /* 4 */) {
	uint32_t chars;
	for (size_t i = 2; i >= 0; --i) {
		chars <<= 8;
		chars |= in[i];
	}
	for (size_t j = 3; j >= 0; --j) {
		out[j] = b64_tab[chars & 0x3F];
		chars >>= 6;
	}
	return 0;
}

int base64_do_in(const char* from, const char* from_end, const char *& from_next,
			SRC::uchar* to, SRC::uchar* to_end, SRC::uchar*& to_next) {
	char ibuf[4];
	SRC::uchar obuf[3];
	const char* from4 = from;
	size_t ipos = 0; // offset in ibuf where next char should be placed
	while ((from != from_end || ipos == 4) && to != to_end) {
		if (ipos == 4) {
			ipos = 0;
			int dret = do_in_i4o3(ibuf, obuf);
			if (dret < 0) {
				if (dret < -1) { // too many equal signs
					from += 2;
					from_next = from;
					to_next = to;
					return -1;
				}
			} else /* (dret >= 0) */ {
				if (std::distance(to, to_end) < dret) {
					break;
				}
				std::copy(obuf, obuf + dret, to);
				from4 = from;
				to += dret;
				if (dret < 3)
					break;
			}
		} else /* (ipos != 4) */ {
			int v = b64_val(*from);
			if (v >= -1) {
				ibuf[ipos++] = *from++;
			} else {
				++from;
			}
		}
	}	
	from_next = from4;
	to_next = to;

	if (from == from_end)
		return 0;
	else
		return 1;
}

}

MOSH_FCGI_BEGIN

namespace http {

u_string Base64::in(const char* in, const char* in_end, const char*& in_next) const {
	u_string str;
	str.resize(std::distance(in, in_end));
	uchar* str_beg = &(*str.begin());
	uchar* str_end = &(*str.end());
	uchar* str_next;
	base64_do_in(in, in_end, in_next, str_beg, str_end, str_next);
	str.resize(std::distance(str_beg, str_next));
	return str;
}

}

MOSH_FCGI_END


