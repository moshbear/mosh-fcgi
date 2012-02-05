#include <locale>
#include <cwchar>
#include <cstdint>
#include <mosh/fcgi/bits/namespace.hpp>
#include <mosh/fcgi/bits/native_utf.hpp>
#include <mosh/fcgi/bits/types.hpp>

using std::mbstate_t;
using std::size_t;

namespace {

/* The following types and functions are for UTF-16 detection and formatting.
 * To ensure full UTF-16 compliance, we ensure that we can work with surrogate
 * pairs, as needed.
 */
enum Native_utf { utf16, utf32 };

enum Surrogate_id { hi, lo, neither };

struct Utf16_pair {
	uint16_t hi;
	uint32_t lo;
	enum { ok, err, noconv } err;
};

template <size_t wc_size> Native_utf native_utf();
template<> Native_utf native_utf<2>() { return utf16; }
template<> Native_utf native_utf<4>() { return utf32; }

Surrogate_id surrogate(wchar_t wc) {
	return
		(wc >= 0xD800 && wc <= 0xDBFF)
			? hi
		: ( wc >= 0xDC00 && wc <= 0xDFFF)
			? lo
		: neither;
}

Utf16_pair to_surrogate(uint32_t wc) {
	Utf16_t pair p;
	if (wc < 0x10000) {
		p.err = Utf16_pair::noconv;
		return p;
	}
	if (wc > 0x10FFFF) {
		p.err = Utf16_pair::err;
		return p;
	}
	p.err = Utf16_pair::ok;
	p.hi = 0xD800;
	p.lo = 0xDC00;
	wc -= 0x10000;
	p.lo |= (wc & 0x3FF);
	wc >>= 10;
	p.hi |= wc; 
	return ret;
}

uint32_t from_surrogate(Utf16_pair p) {
	uint32_t ret;
	p.hi -= 0xD800;
	p.lo -= 0xDC00;
	ret = p.hi;
	ret <<= 10;
	ret += p.lo;
	ret += 0x10000;
	return ret;

}

bool is_mb_cont(unsigned char ch) { return ((ch & 0xC0) == 0x80); /* 10xxxxxx */ }

int in_needbytes(unsigned char ch) {
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
		/* if (cesu8)
		 * 	return 6;
		 * else
		 */	return 4;
	}
	if (wc > 0x07FF)
		return 3;
	if (wc > 0x007F)
		return 2;
	/* if (utf8_mod && (wc  == 0))
	 * 	return 2; // C0 80
	 */
	return 1;
}

unsigned char setch_lead(unsigned char ch, int shift) {
	switch (shift) {
	case 0:  return (ch);
	case 1:  return (ch | 0xC0);
	case 2:  return (ch | 0xE0);
	case 3:  return (ch | 0xF0);
	default: return 0xFF;
	}
}

unsigned char setch_mb(unsigned char ch) { return (0x80 | ch); }

template <typename InputIterator>
bool in_range(InputIterator i, InputIterator j, InputIterator in) {
	while (i != j) {
		if (i == in)
			return true;
		++i;
	}
	return false;
}

// Preconditions: [to, to + shift] is accessible
int u8_encode(wchar_t ch, unsigned char* to) {
	int shift = out_needbytes(ch);
	int shf = shift;
	if (shift < 0)
		return -1;
	unsigned char* end = to + shift;
	while (shift > 0) {
		*end = setch_mb(ch & 0x3F);
		ch >>= 6;
		--end;
		--shift;
	}
	*to = setch_lead(*to, shf);
	return 0;
}

}

MOSH_FCGI_BEGIN

int utf8_in(const uchar* from, const uchar* from_end, const uchar *& from_next,
			wchar_t* to, wchar_t* to_end, wchar_t*& to_next) 
{
	Native_utf my_utf = native_utf<sizeof(wchar_t)>();
	
	while (from != from_end && to != to_end) {
		// simultaneously check for valid starting char
		// and get the trailing byte count
		int shift = in_needbytes(*from);
		if (shift < 0) {
			from_next = from;
			to_next = to;
			return -EILSEQ;
		}
		// is there enough src bytes?
		if (in_range(from, from + shift, from_end))
			break;
		// is there enough dst wchars?
		if (my_utf == utf16 && (shift == 3)) {
			// is there enough room in dst for a surrogate pair?
			if (in_range(to, to + 2, to_end))
				break;
		}  else {
			if (in_range(to, to + 1, to_end))
				break;
		}
		// we work with 32-bit code points so that we can work with [U+0000, U+10FFFF]
		// without the overhead of manipulating surrogate pairs
		uint32_t cp = getch_lead(*from, shift);
		while (shift > 0) {
			if (!is_mb_cont(*++from)) {
				from_next = from;
				to_next = to;
				return -EILSEQ;
			}
			cp <<= 6;
			cp |= getch_mb(*from);
			--shift;
		}
		if (my_utf == utf16) {
			if (cp >= 0x10000) {
				Utf16_pair surr = to_surrogate(cp);
				*to = surr.hi;
				*++to = surr.lo;
			} else
				*to = static_cast<wchar_t>(cp);
		} else {
			*to = cp;
		}
		++to; // either this or replacing *to with *to++
	}
	from_next = from;
	to_next = to;
	if (from == from_end)
		return 0;
	else
		return 1;
}

int utf8_out(const wchar_t* from, const wchar_t* from_end, const wchar_t *& from_next,
		uchar* to, uchar* to_end, uchar *& to_next) 
{
	Native_utf my_utf = native_utf<sizeof(wchar_t)>();
out_loop:
	while (from != from_end && to != to_end) {
		uint32_t cp;
		Utf16_pair p;
		if (my_utf == utf16
		&& (is_surrogate(*from) == hi) && !in_range(from, from + 1, to_end)) {
			p.hi = *from;
			p.lo = *++from;
			if (is_surrogate(p.lo) != lo) {
				from_next = from;
				to_next = to;
				return -EILSEQ;
			}
			/* if (cesu8) { 
			 *	if (in_range(to, to + 5, to_end))
			 *		break;
			 *	u8_encode(p.hi, to);
			 *	u8_encode(p.lo, to + 3);
			 *	++from;
			 *	to += 6;
			 *	goto out_loop;
			 * } else */
				cp = from_surrogate(p);
		} else
			cp = *from;
		++from;
		if (in_range(to, to + out_needbytes(cp), to_end))
			break;
		if ((u8_encode(cp, to)) == -1) {
			from_next = from;
			to_next = to;
			return -EILSEQ;
		}
	}
	from_next = from;
	to_next = to;
	if (from_end == from)
		return 1;
	else
		return 0;

}

std::size_t utf8_length(const uchar* from, const uchar* from_end, std::size_t limit) {
	size_t len = 0;
	Native_utf my_utf = native_utf<sizeof(wchar_t)>();
	while (from != from_end && len < limit) {
		int shift = in_needbytes(*from);
		// found bad character
		if (shift < 0)
			break;
		if (in_range(from, from + shift, from_end))
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
