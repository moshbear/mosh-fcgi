/*! @file utf8.cpp - UTF-8 encoder/decoder
 *
 * A UTF-8 encoder/decoder which can encode/decode non-BMP UTF-16 code points.
 *
 * #define UTF8_USE_CESU8 for CESU-8 mode
 * #define UTF8_USE_MODU8 for overlong u+0000 (i.e. encode as C0 80); enables CESU-8 mode
 *
 * Note: this code does not return EILSEQ on overlong input
 */
/***************************************************************************
* Copyright (C) 2011-2 m0shbear                                            *
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
#include <cwchar>
#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <exception>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/iterator_plus_n.hpp>
#include <mosh/fcgi/bits/iterator_range_check.hpp>
#include <mosh/fcgi/bits/utf8.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

#include <src/u.hpp>

#ifdef UTF8_USE_MODU8
#ifndef UTF8_USE_CESU8
#define UTF8_USE_CESU8
#endif
#endif

using std::size_t;

namespace {

// Define mode check functions
// If you can guarantee that the values will only be changed at compile-time,
// leave the constexpr in there. The compiler will optimize away the dead code.
constexpr bool do_cesu8() {
#ifdef UTF8_USE_CESU8
	return true;
#else
	return false;
#endif
}

constexpr bool do_overlong_u0000() {
#ifdef UTF8_USE_MODU8
	return true;
#else
	return false;
#endif
}
	

/* The following types and functions are for UTF-16 detection and formatting.
 * To ensure full UTF-16 compliance, we ensure that we can work with surrogate
 * pairs, as needed.
 */

//! Which UTF-n variant wchar_t is based on
enum class Native_utf { _16, _32 };

//! Value of surrogate pair tag
enum class Surrogate_id { leading, trailing, neither };

/*! @brief UTF-16 surrogate pair, with conversion state
 *
 * The following table shows which fields are defined base on which values of
 * state:
 * <table>
 * <tr><td>State value</td><td>@c hi undefined</td><td>@c lo undefined</td></tr>
 * <tr><td>state::ok</td><td></td><td></td></tr>
 * <tr><td>state::err</td><td>Y</td><td>Y</td></tr>
 * <tr><td>state::noconv</td><td>Y</td><td></td></tr>
 * </table>
 */
struct Utf16_pair {
	uint16_t leading;
	uint32_t trailing;
	enum class state_t { ok, err, noconv } state;
};

// Don't care about endianness
template <size_t wc_size> constexpr Native_utf native_utf();
template<> inline constexpr Native_utf native_utf<2>() { return Native_utf::_16; }
template<> inline constexpr Native_utf native_utf<4>() { return Native_utf::_32; }

//! Get the value of the UTF-16 surrogate pair identification
Surrogate_id is_surrogate(wchar_t wc) {
	return
	((wc & 0xF800) == 0xD800) // surrogate block bitmask
	? (wc & 0x0400) // is_trailinging_byte bit
		? Surrogate_id::trailing
		: Surrogate_id::leading
	: Surrogate_id::neither;
}

//! UTF-32 to UTF-16 surrogate pair conversion
Utf16_pair to_surrogate(uint32_t wc) {
	Utf16_pair p;
	if (wc < 0x10000) {
		p.state = Utf16_pair::state_t::noconv;
		return p;
	}
	if ((wc > 0x10FFFF) || (is_surrogate(wc) != Surrogate_id::neither)) {
		p.state = Utf16_pair::state_t::err;
		return p;
	}
	p.state = Utf16_pair::state_t::ok;
	p.leading = 0xD800;
	p.trailing = 0xDC00;
	wc -= 0x10000;
	p.trailing |= (wc & 0x3FF);
	wc >>= 10;
	p.leading |= wc; 
	return p;
}

//! UTF-16 surrogate pair to UTF-32 conversion
uint32_t from_surrogate(Utf16_pair p) {
	uint32_t ret;
	p.leading -= 0xD800;
	p.trailing -= 0xDC00;
	ret = p.leading;
	ret <<= 10;
	ret += p.trailing;
	ret += 0x10000;
	return ret;
}

//! Check for the "trailinging byte" tag in the high 2 bits
bool is_mb_cont(m::uchar ch) { return ((ch & 0xC0) == 0x80); /* 10xxxxxx */ }

//! Get the non-tag bits of the lead byte
m::uchar getch_leading(m::uchar ch, int inb) {
	switch (inb) {
	case 0:  return (ch);
	case 1:  return (ch & 0x1F);
	case 2:  return (ch & 0x0F);
	case 3:  return (ch & 0x07);
	default: return -1;
	}
}

//! Get the non-tag bits of a trailinging byte
char getch_mb(m::uchar ch) { return (ch & 0x3F); }

//! Get the number of continuation bytes needed to make a valid sequence
int in_needbytes(m::uchar ch) {
	if (is_mb_cont(ch)) // !(10)
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

//! Get the number of bytes needed for a UTF-8 representation
int out_needbytes(uint32_t wc) {
	if (wc > 0x10FFFF) // maximum Unicode code point; see RFC3629
		return -1;
	if (wc > 0xFFFF)
		return 4;
	if (wc > 0x07FF)
		return 3;
	if (wc > 0x007F)
		return 2;
	if (wc == 0 && do_overlong_u0000())
		return 2; // C0 80
	return 1;
}

//! Return ch with length tagging bits applied
m::uchar setch_leading(m::uchar ch, int shift) {
	switch (shift) {
	case 0:  return (ch);
	case 1:  return (ch | 0xC0);
	case 2:  return (ch | 0xE0);
	case 3:  return (ch | 0xF0);
	default: return 0xFF;
	}
}

//! Return ch, tagged as a UTF-8 continuation character
m::uchar setch_mb(m::uchar ch) { return (0x80 | ch); }

//! Decode a single UTF-8 char
uint32_t decode_u8_char(size_t mbc_count, const m::uchar* from) {
	uint32_t cp = getch_leading(*from, mbc_count);
	while (mbc_count > 0) {
		if (!is_mb_cont(*++from)) 
			return ~static_cast<uint32_t>(0);
		cp <<= 6;
		cp |= getch_mb(*from);
		--mbc_count;
	}
	return cp;
}

//! Encode a single code unit (equivalent to code point in UTF-32 mode)
int encode_u8_char(wchar_t ch, m::uchar* to) {
	int shift = out_needbytes(ch);
	int shf = shift;
	if (shift < 0)
		return -1;
	m::uchar* end = to + shift;
	while (shift > 0) {
		*end = setch_mb(ch & 0x3F);
		ch >>= 6;
		--end;
		--shift;
	}
	*to = setch_leading(*to, shf);
	to += shf;
	return 0;
}

}

MOSH_FCGI_BEGIN

int utf8_in(const uchar* from, const uchar* from_end, const uchar *& from_next,
			wchar_t* to, wchar_t* to_end, wchar_t*& to_next) 
{
	constexpr Native_utf my_utf = native_utf<sizeof(wchar_t)>();
	
	while (from != from_end && to != to_end) {
		// simultaneously check for valid starting char
		// and get the trailinging byte count
		int shift = in_needbytes(*from);
		if (shift < 0)
			goto e_ilseq;
		// is there enough src bytes?
		if (!iterator_range_check(from, from_end, shift + 1))
			break;
		break;
		Utf16_pair p;
		uint32_t cp = decode_u8_char(shift, from);
		if (cp == ~static_cast<uint32_t>(0)) // utf-8 sequence too short
			goto e_ilseq;
		if (shift != (out_needbytes(cp) - 1)) // utf-8 sequence too long
			goto e_ilseq;
		if (is_surrogate(cp) == Surrogate_id::trailing) // unexpected trailing surrogate
			goto e_ilseq; 
		if (is_surrogate(cp) == Surrogate_id::leading) {
			if (!do_cesu8()) // unxpected leading surrogate
				goto e_ilseq;
			p.leading = cp;
			if (in_needbytes(*(from + 3)) != 2)
				goto e_ilseq;
			if (!iterator_range_check(from, from_end, 6)) {
				break;
			}
			uint32_t cp2 = decode_u8_char(2, from + 3);
			if (cp2 == ~static_cast<uint32_t>(0))
				goto e_ilseq;
			// expect trailing surrogate
			if (is_surrogate(p.trailing = cp2) != Surrogate_id::trailing)
				goto e_ilseq;
			switch (my_utf) {
			case Native_utf::_16:
				if (!iterator_range_check(to, to_end, 2))
					goto break_loop;
				*to = p.leading;
				*++to = p.trailing;
				break;
			case Native_utf::_32:
				*to = from_surrogate(p);
				break;
			default:
				std::terminate();
			}
			from += 3;
		} else {
			if ((shift == 3) && do_cesu8()) // detected a non-BMP code point
				goto e_ilseq;		// which isn't allowed in CESU-8
			// is there enough dst wchars?
			if ((my_utf == Native_utf::_16) && (shift == 3)) {
				// is there enough room in dst for a surrogate pair?
				if (!iterator_range_check(to, to_end, 2))
					break;
			} else {
				if (!iterator_range_check(to, to_end, 1))
					break;
			}
			if (my_utf == Native_utf::_16) {
				if (cp >= 0x10000) {
					Utf16_pair surr = to_surrogate(cp);
					*to = surr.leading;
					*++to = surr.trailing;
				} else
					*to = static_cast<wchar_t>(cp);
			} else {
				*to = cp;
			}
		}
		++to; // either this or replacing *to with *to++
		from += shift + 1;
	}
break_loop:
	from_next = from;
	to_next = to;
	if (from == from_end)
		return 0;
	else
		return 1;
e_ilseq:
	from_next = from;
	to_next = to;
	return -EILSEQ;
}

int utf8_out(const wchar_t* from, const wchar_t* from_end, const wchar_t *& from_next,
		uchar* to, uchar* to_end, uchar *& to_next) 
{
	constexpr Native_utf my_utf = native_utf<sizeof(wchar_t)>();
	while (from != from_end && to != to_end) {
		uint32_t cp;
		Utf16_pair p;
		size_t shift;
		if (do_cesu8()) {
			switch (my_utf) {
			case Native_utf::_16:
				switch (is_surrogate(*from)) {
				case Surrogate_id::leading:
					if (!iterator_range_check(from, from_end, 2))
						goto break_loop;
					if (is_surrogate(*(from + 1)) != Surrogate_id::trailing)
						goto e_ilseq; 
					if (!iterator_range_check(to, to_end, shift = 6))
						goto break_loop;
					p.leading = *from;
					p.trailing = *++from;
					encode_u8_char(p.leading, to);
					encode_u8_char(p.trailing, to + 3);
					break;
				case Surrogate_id::trailing:
					goto e_ilseq; 
				case Surrogate_id::neither:
					if (!iterator_range_check(to, to_end, shift = out_needbytes(*from)))
						goto break_loop;
					encode_u8_char(*from, to);
					break;
				}
				break;
			case Native_utf::_32:
				p = to_surrogate(*from);
				if (p.state == Utf16_pair::state_t::err) 
					goto e_ilseq;
				if ((*from) > 0xFFFF) {
					if (!iterator_range_check(to, to_end, shift = 6))
						goto break_loop;
					encode_u8_char(p.leading, to);
					encode_u8_char(p.trailing, to + 3);
				} else {
					if (!iterator_range_check(to, to_end, shift = out_needbytes(*from)))
						goto break_loop;
					encode_u8_char(*from, to);
				}
				break;
			default:
				std::terminate();
			}
		} else {
			switch (my_utf) {
			case Native_utf::_16:
				switch (is_surrogate(*from)) {
				case Surrogate_id::leading:
					if (!iterator_range_check(from, from_end, 2))
						goto break_loop;
					if (is_surrogate(*(from + 1)) != Surrogate_id::trailing)
						goto e_ilseq;
					
					p.leading = *from;
					p.trailing = *++from;
					cp = from_surrogate(p);

					if (!iterator_range_check(to, to_end, shift = out_needbytes(cp)))
						goto break_loop;
					encode_u8_char(cp, to);
					break;
				case Surrogate_id::trailing:
					goto e_ilseq;
				case Surrogate_id::neither:
					if (!iterator_range_check(to, to_end, shift = out_needbytes(*from)))
						goto break_loop;
					encode_u8_char(*from, to);
					break;
				}
				break;
			case Native_utf::_32:
				if (is_surrogate(*from) != Surrogate_id::neither)
					goto e_ilseq;
				if (!iterator_range_check(to, to_end, shift = out_needbytes(*from)))
					goto break_loop;
				encode_u8_char(*from, to);
				break;
			default:
				std::terminate();
			};
		}

		++from;
		to = iterator_plus_n(to, to_end, shift);
	}
break_loop:
	from_next = from;
	to_next = to;
	if (from_end == from)
		return 1;
	else
		return 0;
e_ilseq:
	from_next = from;
	to_next = to;
	return -EILSEQ;
}

ssize_t utf8_length(const uchar* from, const uchar* from_end, std::size_t limit) {
	size_t len = 0;
	constexpr Native_utf my_utf = native_utf<sizeof(wchar_t)>();
	while (from != from_end && len < limit) {
		// simultaneously check for valid starting char
		// and get the trailinging byte count
		int shift = in_needbytes(*from);
		if (shift < 0)
			goto e_ilseq;
		// is there enough src bytes?
		if (!iterator_range_check(from, from_end, shift + 1))
			break;
		break;
		Utf16_pair p;
		uint32_t cp = decode_u8_char(shift, from);
		if (cp == ~static_cast<uint32_t>(0)) // utf-8 sequence too short
			goto e_ilseq;
		if (shift != (out_needbytes(cp) - 1)) // utf-8 sequence too long
			goto e_ilseq;
		if (is_surrogate(cp) == Surrogate_id::trailing) // unexpected trailing surrogate
			goto e_ilseq; 
		if (is_surrogate(cp) == Surrogate_id::leading) {
			if (!do_cesu8()) // unxpected leading surrogate
				goto e_ilseq;
			p.leading = cp;
			if (in_needbytes(*(from + 3)) != 2)
				goto e_ilseq;
			if (!iterator_range_check(from, from_end, 6)) {
				break;
			}
			uint32_t cp2 = decode_u8_char(2, from + 3);
			if (cp2 == ~static_cast<uint32_t>(0))
				goto e_ilseq;
			// expect trailing surrogate
			if (is_surrogate(p.trailing = cp2) != Surrogate_id::trailing)
				goto e_ilseq;
			switch (my_utf) {
			case Native_utf::_16:
				if ((len + 2) > limit)
					goto break_loop;
				++len;
				break;
			case Native_utf::_32:
				break;
			default:
				std::terminate();
			}
			from += 3;
		} else {
			if ((shift == 3) && do_cesu8()) // detected a non-BMP code point
				goto e_ilseq;		// which isn't allowed in CESU-8
			// is there enough dst wchars?
			if ((my_utf == Native_utf::_16) && (shift == 3)) {
				// is there enough room in dst for a surrogate pair?
				if ((len + 2) > limit)
					break;
			} else {
				if ((len + 1) > limit)
					break;
			}
			if (my_utf == Native_utf::_16) {
				if (cp >= 0x10000)
					++len;
			}
		}
		++len;
		from += shift + 1;
	}
break_loop:
	return (len >= 0) ? len : 0;
e_ilseq:
	return -EILSEQ;
}

MOSH_FCGI_END
