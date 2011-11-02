//! @file utf8_cvt.cpp UTF-8 codecvt facet
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
#include <algorithm>
#include <locale>
#include <cstdint>
#include <cwchar>
#include <mosh/fcgi/bits/utf8_cvt.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

using std::size_t;
using std::codecvt_base;
using std::codecvt;

namespace {

/* The following types and functions are for UTF-16 detection and formatting.
 * To ensure full UTF-16 compliance, we ensure that we can work with surrogate
 * pairs, as needed.
 */
enum Native_utf { utf16, utf32 };

enum Surrogate_id { surr_hi, surr_lo, surr_neither };

struct Utf16_pair {
	uint16_t surr_hi;
	uint32_t surr_lo;
	enum { ok, error, noconv } err;
};

template <size_t wc_size> Native_utf native_utf();
template<> Native_utf native_utf<2>() { return utf16; }
template<> Native_utf native_utf<4>() { return utf32; }

Surrogate_id surrogate(wchar_t wc) {
	return
		(wc >= 0xD800 && wc <= 0xDBFF)
			? surr_hi
		: ( wc >= 0xDC00 && wc <= 0xDFFF)
			? surr_lo
		: surr_neither;
}

Utf16_pair to_surrogate(uint32_t wc) {
	Utf16_pair p;
	if (wc < 0x10000) {
		p.err = Utf16_pair::noconv;
		return p;
	}
	if (wc > 0x10FFFF) {
		p.err = Utf16_pair::error;
		return p;
	}
	p.err = Utf16_pair::ok;
	p.surr_hi = 0xD800;
	p.surr_lo = 0xDC00;
		wc -= 0x10000;
		p.surr_lo |= (wc & 0x3FF);
		wc >>= 10;
		p.surr_hi |= wc; 
	return p;
}

uint32_t from_surrogate(Utf16_pair p) {
	uint32_t ret;
	p.surr_hi -= 0xD800;
	p.surr_lo -= 0xDC00;
	ret = p.surr_hi;
	ret <<= 10;
	ret += p.surr_lo;
	ret += 0x10000;
	return ret;

}

bool is_mb_cont(char ch) { return ((ch & 0xC0) == 0x80); /* 10xxxxxx */ }

int in_needbytes(unsigned char ch) {
	if (is_mb_cont(ch)) // !(10)
		return -1;
	if (ch >= 0xF5) // 3 trailing bytes encoding a code point above U+10FFFF	
		return -1;
	if (ch <= 0x7F) // 0 (ASCII)
		return 0;
	if ((ch & 0xE0) == 0xC0) // 110
		return 1;
	if ((ch & 0xF0) == 0xE0) // 1110
		return 2;
	if ((ch & 0xF8) == 0xF0) // 11110
		return 3;
	return -1; // 11111
}

char getch_lead(unsigned char ch, int inb) {
	switch (inb) {
	case 0:  return (ch);
	case 1:  return (ch & 0x1F);
	case 2:  return (ch & 0x0F);
	case 3:  return (ch & 0x07);
	default: return -1;
	}
}

char getch_mb(unsigned char ch) { return (ch & 0x3F); }

//! Get the number of bytes needed for a UTF-8 representation
int out_needbytes(uint32_t wc) {
	if (wc > 0x10FFFF) // maximum Unicode code point; see RFC3629
		return -1;
	if (wc > 0xFFFF) {
		return 4;
	}
	if (wc > 0x07FF)
		return 3;
	if (wc > 0x007F)
		return 2;
	return 1;
}

unsigned char setch_lead(char ch, int shift) {
	switch (shift) {
	case 0:  return (ch);
	case 1:  return (ch | 0xC0);
	case 2:  return (ch | 0xE0);
	case 3:  return (ch | 0xF0);
	default: return 0xFF;
	}
}

unsigned char setch_mb(char ch) { return (0x80 | ch); }

// Preconditions: [to, to + shift] is accessible
int u8_encode(wchar_t ch, char* to) {
	int shift = out_needbytes(ch);
	int shf = shift;
	if (shift < 0)
		return -1;
	char* end = to + shift;
	while (shift > 0) {
		*end = setch_mb(ch & 0x3F);
		ch >>= 6;
		--end;
		--shift;
	}
	*to = setch_lead(static_cast<unsigned char>(*to), shf);
	return 0;
}

}

MOSH_FCGI_BEGIN

codecvt_base::result Utf8_cvt::do_in(
			std::mbstate_t&, const char* from,
			const char* from_end, const char *& from_next,
			wchar_t* to, wchar_t* to_end, wchar_t*& to_next
		) const
{
	Native_utf my_utf = native_utf<sizeof(wchar_t)>();
	
	while (from != from_end && to != to_end) {
		// simultaneously check for valid starting char
		// and get the trailing byte count
		int shift = in_needbytes(*from);
		if (shift < 0) {
			from_next = from;
			to_next = to;
			return codecvt_base::error;
		}
		// is there enough src bytes?
		if (std::distance(from, from_end) <= shift)
			break;
		// is there enough dst wchars?
		if (my_utf == utf16 && (shift == 3)) {
			// is there enough room in dst for a surrogate pair?
			if (std::distance(to, to_end) <= 1)
				break;
		}
		// we work with 32-bit code points so that we can work with [U+0000, U+10FFFF]
		// without the overhead of manipulating surrogate pairs
		uint32_t cp = getch_lead(*from, shift);
		while (shift > 0) {
			if (!is_mb_cont(*++from)) {
				from_next = from;
				to_next = to;
				return codecvt_base::error;
			}
			cp <<= 6;
			cp |= getch_mb(*from);
			--shift;
		}
		// space check done at :171
		if (my_utf == utf16) {
			if (cp >= 0x10000) {
				Utf16_pair surr = to_surrogate(cp);
				*to++ = surr.surr_hi;
				*to++ = surr.surr_lo;

			} else
				*to++ = static_cast<wchar_t>(cp);
		} else {
			*to++ = cp;
		}
	}
	from_next = from;
	to_next = to;
	if (from == from_end)
		return codecvt_base::ok;
	else
		return codecvt_base::partial;
}

codecvt_base::result Utf8_cvt::do_out(
			std::mbstate_t&, const wchar_t* from,
			const wchar_t* from_end, const wchar_t *& from_next,
			char* to, char* to_end, char *& to_next) const
{
	Native_utf my_utf = native_utf<sizeof(wchar_t)>();
	while (from != from_end && to != to_end) {
		uint32_t cp;
		Utf16_pair p;
		if (my_utf == utf16
		&& (surrogate(*from) == surr_hi) && (std::distance(from, from_end) >= 2)) {
			p.surr_hi = *from;
			p.surr_lo = *(from + 1);
			if (surrogate(p.surr_lo) != surr_lo) {
				++from;
u8_cvt_do_out_err:		from_next = from;
				to_next = to;
				return codecvt_base::error;
			}
			cp = from_surrogate(p);
		} else
			cp = *from;
		if (out_needbytes(cp) == -1) 
			goto u8_cvt_do_out_err;
		if (std::distance(to, to_end) < out_needbytes(cp))
			break;
		if ((u8_encode(cp, to)) == -1) 
			goto u8_cvt_do_out_err;
		++from; // increment after all the checks so that from_next is loaded
			// with the correct value of from
		to += out_needbytes(cp);
	}
	from_next = from;
	to_next = to;
	if (from == from_end)
		return codecvt_base::ok;
	else
		return codecvt_base::partial;

}

int Utf8_cvt::do_length(std::mbstate_t&, const char* from,
const char* from_end, std::size_t limit) const
{
	int len = 0;
	Native_utf my_utf = native_utf<sizeof(wchar_t)>();
	while (from != from_end && len < limit) {
		int shift = in_needbytes(*from);
		// found bad character
		if (shift < 0)
			break;
		if (std::distance(from, from_end) <= shift)
			break;
		from += shift;
		// Increment len by 2 if we're dealing with UTF-16 and
		// input is in [U+10000, 10FFFF]
		if (my_utf == utf16 && (shift == 3))
			len += 2; 
		else
			len += 1;
	}
	if (len == limit + 1)
		len -= 2;
	if (len < 0)
		len = 0;
	return len;
}

MOSH_FCGI_END
