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
	// no pattern here, so use cases
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
			// replace spaces in newly-appended portion of string
			std::replace_if(str.begin() + _n, str.end(), [=] (uchar ch) { return ch == '+'; }, ' ');
			from = pct;
		} else {
			if (std::distance(from, from_end) > 2) {
				int hi = *(from + 1); // high nibble
				int lo = *(from + 2); // low nibble
				if (std::isxdigit(hi) && std::isxdigit(lo)) {
					str += static_cast<SRC::uchar>(          // Case-insensitive hex decoding:
							(((hi & 0x40)            // If we have an alphabetic character,
								? ((hi & 7) + 9) // fetch the three value bits and add
								: (hi & 0x0F)    // 9, since 'A' is 0x41, not 0x40.
							)                        // Otherwise, we have a numeric
							<< 4) |                  // character, so fetch the four value
							((lo & 0x40)             // bits. Case is ignored since we're
								? ((lo & 7) + 9) // not checking against 0x60. 
								: (lo & 0x0F)    // Do this for the first char, shift
							)                        // left by four (due to it being the
						);                               // hi nibble), and bit-or it with the
						                                 // second char.
					from += 2;
				} else
					str += '%';
				++from;
			} else
				break;
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

