//! @file http/mime.cpp MIME parsing (split from http/session.cpp)
/***************************************************************************
* Copyright (C) 2013 m0shbear                                              *
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
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <tuple>
#include <utility>
#include <vector>
#include <cstring>
extern "C" {
#include <iconv.h>
}
#include <mosh/fcgi/http/conv.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/utility.hpp>
#include <src/http/mime.hpp>
#include <src/utf8.hpp>
#include <src/namespace.hpp>


namespace {

#include "lut.hpp"

const std::map<std::string, int> content_types = {
		std::make_pair("8bit", SRC::mime::cte_flags::ign),
		std::make_pair("base64", SRC::mime::cte_flags::def),
		std::make_pair("binary", SRC::mime::cte_flags::ign),
		std::make_pair("quoted-printable", SRC::mime::cte_flags::def)
};


// Match result of find_nonrecursive_comm_at.
//
// To check whether this is a match or a fail, check the value of .ok()
// If true, .choke is the position of where the matcher bailed out and .failcode is the reason why.
// If false, .left is the position of the '(' and .right is the position of the ')'.
// Exception safety is used to prevent UB from occuring.
class Nonrecursive_comm_match {
public:
	typedef std::string::size_type size_type;
	enum fail_state {
		end_of_string,
		bad_lparen,
		not_rightmost,
		syntax_error
	};
	// Safe getters
	size_type left() const
	{ return return_or_throw(xleft, ok(), "error state && get .left"); }
	size_type right() const
	{ return return_or_throw(xright, ok(), "error state && get .right"); }
	size_type choke() const
	{ return return_or_throw(xchoke, !ok(), "non-error state && get .choke"); }
	fail_state failcode() const
	{ return return_or_throw(xfailcode, !ok(), "non-error state && get .failcode"); }
	bool ok() const {
		return static_cast<bool>(return_or_throw(xok, xok.initialized(),
						"Uninitialized error state; check for default init"));
	}
	
	// return an instance of an OK match [%left_,%right_]
	static Nonrecursive_comm_match ok(size_type left_, size_type right_) {
		return Nonrecursive_comm_match(xtrue(), left_, right_);
	}
	// return an instance of a fail match, failing at position %choke_ with reason %code
	// %left and %right are set to %bad to indicate failure
	static Nonrecursive_comm_match fail(size_type choke_, fail_state code) {
		return Nonrecursive_comm_match(xfalse(), choke_, code);
	}
private:
	struct xtrue { };
	struct xfalse { };

	Nonrecursive_comm_match() = delete;
	
	Nonrecursive_comm_match(xtrue, size_type left_, size_type right_)
	: xleft(left_), xright(right_), xok(true) { }
	Nonrecursive_comm_match(xfalse, size_type choke_, fail_state code)
	: xchoke(choke_), xfailcode(code), xok(false) { }

	size_type xleft;
	size_type xright;
	size_type xchoke;
	fail_state xfailcode;

	// Helpers for safe getters
	// If predicate %pred holds, return a copy of %val, otherwise throw
	// a std::logic_error with string %ex_str.
	template <typename T>
	static T return_or_throw(T const& val, bool pred, const char* ex_str) {
		if (pred)
			return val;
		throw std::logic_error(ex_str);
	}
	// optional<bool> without the overhead of optional<T>
	struct Optional_bool {
		int p;
		Optional_bool() : p(0) { }
		Optional_bool(bool b) : p(b ? 2 : 1) { }
		operator bool() const {
			if (!p)
				throw std::runtime_error("can't convert to bool");
			return static_cast<bool>(p - 1);
		}
		bool initialized() const noexcept
		{ return !!p; }
		Optional_bool& operator=(bool b)
		{ p = b ? 2 : 1; return *this; }
	} xok;
};

// Skip nonquoted and backlash-escaped chars.
// Behavior of overloads (for brevity, c = const; size_type = std::string::size_type):
// (1) (std::string c& str, size_type& pos): if true, %pos is either the position of
// 					     the next quotation mark or is equal to
// 					     %str.size(); if false, %pos is the
// 					     position of a backslash corresponding
// 					     to an incomplete quoted-char
// 					     NOTE: code does not check for \r or
// 					           non-ASCII.
// (2) (c char* s, size_type s_size, size_type& pos): like (1), but substitute
// 						      %str.size with %(s + s_size)
// (3) (c char*& s, c char* end): like (2), but substitute %pos with %s and
// 				  %(s + s_size) with %end
// Overloads (1) and (2) perform range checks on %pos to make sure it doesn't refer
// past the last valid char or hold a reserved value.
bool skip_nqb(std::string const& src, std::string::size_type& safe_pos) {
	return skip_nqb(src.data(), src.size(), safe_pos);
}
bool skip_nqb(const char* src_p, std::string::size_type src_size, std::string::size_type& safe_pos) {
	if (safe_pos == std::string::npos || safe_pos >= src_size)
		return false;
	auto const* src_end = src_p + src_size;
	src_p += safe_pos;
	auto const old_p = src_p;
	bool ret = skip_nqb(src_p, src_end);
	safe_pos += (src_p - old_p);
	return ret;
}
bool skip_nqb(const char*& src_p, const char* src_end) {
	for( ; src_p != src_end; ++src_p) {
		if(tokens[widen_cast<int>(*src_p)] & tok_flags::not_quote_or_backslash)
			continue;
		if (*src_p == '\\') {
			if (src_end - src_p > 1) 
				++src_p;
			else
				return false;
		} else
			break;
	}
	return true;
}

// Find the next non-nested comment-block /\(([^()\r\\]|\\.)*\)/ in %src, starting at position %at.
//
// See %Nonrecursive_comm_match to make sense of the returned structure.
//
// Searching does not perform look-behind, with the following exception: nearest '(' has a lookbehind
// peformed on it to check whether it is a valid start-comment-block delimiter.
// 
// %fail_state has 4 possible values, each of which show what kind of handling is further required
//
// %fail_state::end_of_string:		end of string: no '(' found or incomplete quoted-pair (\\.)
// %fail_state::bad_lparen:		backlash precedes nearest '('; search again with %at == %choke + 1
// %fail_state::not_rightmost:		'(' found before ')' found; search again with %at == %choke
// %fail_state::syntax_error:		non-ASCII found or unescaped \r found; caller has liberty to
// 					choose whether to try again with at %choke + 1 or to emit a
// 					parsing or encoding diagnostic
Nonrecursive_comm_match find_nonrecursive_comm_at(std::string const& src, std::string::size_type at) {
	const auto src_p = src.data();
	auto src_size = src.size();
		
	std::string::size_type left, right;
	left = src.find('(', at);
		
	if (left == std::string::npos) 
		return Nonrecursive_comm_match::fail(left, Nonrecursive_comm_match::fail_state::end_of_string);
	if (left > 0 && src_p[left - 1] == '\\')
		return Nonrecursive_comm_match::fail(left, Nonrecursive_comm_match::fail_state::bad_lparen);
	// scan to see how long we can make the \((?:[^()\r\\]|\\.)*\) sequence
	for (right = left + 1; right < src_size; ++right) {
		// Check for invalid token: unescaped [^()\r\\] (\\ follows with ., so length-check that case)
		if (tokens[widen_cast<int>(src_p[right])] & tok_flags::ctext)
			continue;
		
		// non-ASCII; abort parsing
		auto not_ascii = [] (char ch) { int x(widen_cast<int>(ch)); return (x < 0 || x > 0x7F); };
		if (not_ascii(src_p[right]))
			return Nonrecursive_comm_match::fail(right,
				Nonrecursive_comm_match::fail_state::syntax_error);
				
		if (src_p[right] == '\\') {
			if (src_size - right > 1) {
				// non-ASCII; abort parsing
				if (not_ascii(src_p[++right]))
					return Nonrecursive_comm_match::fail(right,
							Nonrecursive_comm_match::fail_state::syntax_error);
			}
			else 
			     return Nonrecursive_comm_match::fail(right,
			     		Nonrecursive_comm_match::fail_state::end_of_string);
		} else { // \r
			
			break;
		}
	}
	// Fall-through of consume-loop because at least one of the following holds:
	// 	(1) end of string reached
	// 	(2) caught \r
	// (1) and (2) can be weakly combined on the basis that char() \not\in [()] 
	// 
	switch (src_p[right]) {
	// Check for rightmost derivation: a ')' terminates the sequence of valid ctext;
	// a '(' means that we have a nested (recursive) comment
	case ')':
		return Nonrecursive_comm_match::ok(left, right);
	case '(':
		return Nonrecursive_comm_match::fail(right, Nonrecursive_comm_match::fail_state::not_rightmost);
	// Also check for other reasons for loop termination: unexpected end of string or (ideally) unbackslashed \r
	default:
		if (right == src_size)
			return Nonrecursive_comm_match::fail(right,
				Nonrecursive_comm_match::fail_state::end_of_string);
		// Syntax error: \r found
		return Nonrecursive_comm_match::fail(right, Nonrecursive_comm_match::fail_state::syntax_error);
	}
}

// Get the list indices of unescaped quote-pairs in %src
std::list<SRC::tuple_rep<2, std::string::size_type>::type> get_quotepairs(std::string const& src) {
	std::list<SRC::tuple_rep<2, std::string::size_type>::type> quote_list;
	std::string::size_type pos = 0;
	auto src_size = src.size();
	decltype(pos) pos2 = pos;
	decltype(pos) quote_pos[] { pos, pos };
	// don't place inside the quote-scanning for-loop - we need to see
	// if we have an open quote till the end of the string after filtering is done
	auto quote_count = 0;
	// skip non-quoted and backlash-escaped parts
	mime_detail::skip_nqb(src, pos2);
	// skip_nqb() stopped because we reached either end of string or hit a "
	if (pos2 == src_size)
		goto ret;
	// search for quoted-strings and remove comment matches where the left bracket
	// is embedded in the quote, since it's then not considered a comment anymore
	pos = pos2;
	while (pos < src_size && src[pos2] == '"') {
		pos2 = pos;
		if ((pos2 > 0 && src[pos2 - 1] != '\\') || pos2 == 0) {
			quote_pos[0] = pos2;
			quote_count = 1;
		} else
			quote_count = 0;
		// find quote-scope range
		++pos2;
		if (!mime_detail::skip_nqb(src, pos2)) {
				break;
		}
		if (pos2 == src_size) {
			break;
		}
		if (src[pos2] != '"') {
			break;
		}
		quote_count = (quote_count + 1) % 2;
		quote_pos[(quote_count + 1) % 2] = pos2;
		// Incomplete quote; try again with this quote as the opening quote
		if (quote_count) {
			pos = pos2;
			continue;
		} else 
			quote_list.push_back(std::make_tuple(quote_pos[0], quote_pos[1]));
		++pos2;
		if (!mime_detail::skip_nqb(src, pos2)) {
			break;
		}
		pos = pos2;
	}	
	// quote parser loop exited because it couldn't find a closing " ->
	// treat the rest of the string as within the quoted-section
	if (quote_count == 1) {
		quote_list.push_back(std::make_tuple(quote_pos[0], std::string::npos));
	}
ret:
	return quote_list;
}

// Get the position of the next character in a string that's not inside a quote block
// %Q_seq_type is the type of the container that holds the list of quote-pair indices; see get_quotepairs()
template <typename Q_seq_type>
class next_ch {
public:
	next_ch(Q_seq_type const& q, std::string const& s, char c, std::string::size_type startpos = 0)
	: state(st::ok), q_it(q.begin()), q_end(q.end()), str(s), ch(c), pos(startpos) : xcomp(false)
	{ }
	virtual ~next_ch() { }
	
	next_ch& next() {
		if (state == st::end_of_string) {
			return *this;
		}
		pos = str.get().find(ch, pos + (xcomp ? 1 : 0));
		if (pos == std::string::npos) {
			state = st::end_of_string;
			return *this;
		}
		// check whether %ch is inside a quoted-string
		if (([&] {
			std::string::size_type l, r;
			while (q_it != q_end) {
				std::tie(l, r) = *(this->q_it);
				// sliding window: have quotes_it increment
				// if start_pos is too far forward
				if (this->pos > r) {
					++(this->q_it);
					continue;
				}
				if (this->pos < l)
					return false;
				// this needs to be before the start_pos <= r comparison since
				// return true -> the next semicolon *may* be useful; if we
				// have an unbalanced quote pair, that's not happening
				if (this->pos >= l && r == std::string::npos) {
					this->state = st::end_of_string;
					return true;
				}
				if (this->pos >= l && this->pos <= r)
					return true;
			}
			return false;
		})()) {
			xcomp = true;
			return this->next();
		}
		if (pos > 0 && (skip_bs && str.get()[pos -  1] == '\\')) {
			xcomp = true;
			return this->next;
		}
		
		xcomp = true;
		return *this;
	}

	next_ch copy() const {
		return *this;
	}

	next_ch& advance_to_pos(std::string::size_type xpos) {
		if (xpos < pos)
			throw std::invalid_argument("xpos < pos; advance_to_pos does not support rewinding");
		
		pos = xpos;
		xcomp = false;
		return *this;
	}

	std::string::size_type get() const noexcept {
		switch (state) {
		case st::ok:
			return pos;
		case st::end_of_string:
			return std::string::npos;
		}
	}

	enum class st {
		ok,
		end_of_string,
	} state;

private:
	std::reference_wrapper<const std::string> str;
	Q_seq_type::const_iterator q_it;
	Q_seq_type::const_iterator q_end;

	char ch;
	std::string::size_type pos;
	bool xcomp;
	bool skip_bs;
};


std::string do_ctype_boundary(std::string const& str) {
	if (str.empty())
		return "";
	// You *can* forgo the lookup table and use std::string::find_first_not_of, but that has a cost of
	// O(nm) time, where m == strlen(BCHAR) and n == str.size().
	// The lookup table reduces this to O(n).
	for (char ch : str) {
		if (!(tokens[SRC::widen_cast<int>(ch)] & tok_flags::bchar))
			return "";
	}
	// No need for LUT: haystack space has at most 70 chars and needle space 2
	return "--" + str.substr(0, [] (std::string::size_type n) { return (n == std::string::npos) ? 70 : n; }
					(str.find_last_not_of(" \t", 69, 2)));
}

struct filtering_param {
	std::pair<SRC::mime::filtering::field, SRC::mime::filtering::subfield_union> id;
	std::function<std::string (std::string const&)> func;
};

constexpr inline filtering_param make_filtering_param(SRC::mime::filtering::field f,
						SRC::mime::filtering::subfield_union subf,
						std::function<std::string (std::string const&)> func) {
	return filtering_param(std::make_pair(f, subf), func);
}

constexpr inline std::pair<decltype(filtering_param::id), filtering_param>
make_filtering_param_entry(SRC::mime::filtering::field f,
			SRC::mime::filtering::subfield_union subf,
			std::function<std::string (std::string const&)> func) {
	return std::make_pair(std::make_pair(f, subf), make_filtering_param(f, subf, func));
}

const std::map<decltype(filtering_param::id), filtering_param> filters = {
	make_filtering_param_entry(SRC::mime::filtering::field::content_type,
				SRC::mime::filtering::subfield_content_type::boundary,
				[&tokens] (std::string const& s) -> std::string { return do_ctype_boundary(s); }),
	make_filtering_param_entry(SRC::mime::filtering::field::content_type,
				SRC::mime::filtering::subfield_content_type::charset,
				[&tokens] (std::string const& s) -> std::string { 
					


}

SRC_BEGIN

namespace mime {

std::string fetch_cte(std::string const& str, int cmp_flag) {
	std::vector<std::string> filter;
	for (auto const& t : content_types) {
		if (t.second & cmp_flag)
			filter.push_back(t.first);
	}

	if (filter.size() == 0)
		return "";

	std::size_t maxlen = std::max_element(filter.begin(), filter.end(), 
						[] (std::string const& a, std::string const& b) {
							return a.size() < b.size();
						})->size();
	std::string istr;
	str.reserve(maxlen);
	
	for (size_t i = 0; i < maxlen; ++i)
		istr += narrow_cast<char>(lower[widen_cast<int>(str[i])]);
	
	for (std::string const& type : filter) {
		if (!istr.compare(0, type.size(), type))
			return type;
	}

	return "";
}

maybe_string::type canonicalize(std::string const& buf) {
	std::string nocont;

	// strip CRLF 1*LSWP
	{
		std::string::size_type old_pos = 0;
		std::string::size_type buf_size = buf.size();
		const char* datap = buf.data();
		do {
			std::string::size_type crlf_pos = buf.find("\r\n", old_pos);
			std::string::size_type nws_pos = std::string::npos;
			// no more newlines or end of buffer - copy data and break from loop
			if (crlf_pos == std::string::npos || crlf_pos + 2 == buf_size) {
				nocont += std::move(buf.substr(oldpos, std::string::npos));
				break;
			}

			nws_pos = crlf_pos + 2;
			for (; nws_pos < buf_size; ++nws_pos) {
				if (!(tokens[widen_cast<int>(datap[nws_pos])] & tok_flags::lwspchar))
					break;
			}
			if (nws_pos == crlf_pos + 2)
				
				crlf_pos += 2; // ensure the \r\n is appended to s
			nocont += std::move(buf.substr(old_pos, crlf_pos - old_pos));
			old_pos = nws_pos;
		} while (old_pos != std::string::npos);
	}

	// strip comments
	
	// Since comments can be arbitrarily nested, we find the innermost comments and strip it, iteratively,
	// then swap buffer pointers and repeat the search until there are no comments (that is, '(' without a
	// preceding opening '"' )
	// Swapping buffer pointers allows us to do multiple passes without string copy
	// overhead.

	std::string nocomm;
	// don't place this inside the for-loop; we need the value of the pointer so we know which
	// string to return
	std::string* dst;
	for (unsigned pass = 1 ;; ++pass) {
		using C_match = Nonrecursive_comm_match;

		std::string const* src;

		if (pass % 2) {
			dst = &nocomm;
			src = &nocont;
		} else {
			dst = &nocont;
			src = &nocomm;
		}
		// source string offset
		std::string::size_type src_pos = 0;
		// source string size
		std::string::size_type src_size = src->size();
		// safe substring position
		const char* src_p = src->data();

		// Generate a list of potential matches for this pass.
		// Later code that parses quotes will remove candidates from the list if there's a
		// preceding opening-quote, since quoted-string takes precedence over comments, on the basis that
		// comment fields don't have quoted-strings (i.e. " inside a comment does not have semantic
		// significance).
		std::list<SRC::tuple_rep<2, C_match::size_type>> potential;

		for (decltype(src_size) pos = 0; pos < src_size; ++pos) {
			C_match p(find_nonrecursive_comm_at(*src, pos));
			// Found a match; add it to the list and increment the position
			// so we don't scan overlaps and duplicate our work (scanning happens from the first
			// found '(', we're duplicating work if we don't skip past the found sequence
			if (p.ok()) {
				potential.push_back(std::make_tuple(p.left(), p.right()));
				pos = p.right();
				continue;
			}
			auto const& choke = p.choke();
			switch (p.failcode()) {
			// No ')' found in [pos, src->size()) or
			// the last character is a '\\' (i.e. an incomplete quoted-pair),
			// which means we're done parsing
			case C_match::fail_state::end_of_string:
				goto psearch_break;
			// Bad lparen implies that the next '(' is backslash-escaped;
			// try searching from that position onward
			case C_match::fail_state::bad_lparen:
				pos = choke;
				break;
			// Not a rightmost derivation (we found a following '(' before finding a matching ')';
			// shift to the location of the following '(' and try again
			case C_match::fail_state::not_rightmost:
				pos = choke - 1;
				goto psearch_continue;
			// Unescaped \r or non-ASCII detected; invoke bad-char error handling if non-ASCII
			// Defer the parser-error handling for after quote parsing
			case C_match::fail_state::syntax_error:
				if (src_p[choke] != '\r')
					return maybe_string:error("Bad char detected: "
							+ char_to_hex_string(src_p[choke]));
				break;
			default: ;
			}
		// the _continue and _break labels are here so that we can break out of the for(){} scope
		// while within switch(){} scope
			psearch_continue: ; 
		}
		psearch_break: ;
		
		{	
			auto quotes = get_quotepairs(*src);

			decltype(potential)::iterator p_it = potential.begin();
			decltype(potential)::iterator p_end = potential.end();

			decltype(quotes)::const_iterator q_it = quotes.begin();
			decltype(quotes)::const_iterator q_end = quotes.end();


			std::string::size_type bkt_l, bkt_r;
			std::string::size_type qp_l, qp_r;
				
			while ((q_it != q_end)
			&& ([&] () {
				std::tie(qp_l, qp_r) = *q_it++;
				return qp_r != std::string::npos;
			})()) {
				while ((p_it != p_end)
				&& ([&] () {
					std::tie(bkt_l, bkt_r) = *p_it;
					bool pred = bkt_l < qp_l;
					if (!pred)
						return pred;
					++p_it;
					return pred;
				})());
				
				while ((p_it != p_end)
				&& ([&] () {
					std::tie(bkt_l, bkt_r) = *p_it;
					bool pred = bkt_l < qp_r;
					if (!pred)
						return pred;
					if (std::next(p_it) == p_end) {
						p_end = p_it;
						potential.erase(p_it);
					} else {
						potential.erase(p_it++);
					}
					return pred;
				})());	
			}
			
			if (std::next(q_it) == q_end && qp_r == std::string::npos) {
				while ((p_it != p_end)
				 && ([&] () {
				 	std::tie(bkt_l, bkt_r) = *p_it;
					return true;
				}())) {
			 		if (qp_l < bkt_l) {
						 // %potential is in sorted order, so we can split the list at this position
			   			 // and destroy all remaining ones without comparing
			     			 decltype(potential) p2;
			     			 p2.splice(p2.begin(), potential, p_it, p_end);
			       			 p_end = p_it;
					 } else
						 ++p_it;
				}
			}
		}

	do_extract:
		// no comments to filter out -> we're done here
		if (potential.empty()) {
			*dst = std::move(*src);
			break;
		// copy 
		} else {
			decltype(potential)::const_iterator p_it = potential.begin();
			decltype(potential)::const_iterator p_end = potential.end();
			*dst = src->substr(0, std::get<0>(*p_it);
			for (; ++p_it != p_end; ) {
				auto last_right = std::get<1>(*std::prev(p_it));
				*dst += ' ';
				*dst += src->substr(last_right + 1, std::get<0>(*p_it) - last_right - 1);
			}
			xassert(p_it == p_end,
				"Iterator sanity check: p_it != p_end [violation of preceding for-loop]");
			*dst += ' ';
			*dst += src->substr(std::get<1>(*std::prev(p_it)) + 1, std::string::npos);
		}
	}
	return maybe_string::ok(*dst);
}

std::map<std::string, std::string> get_mime_params(std::string const& header_line) {
	std::map<std::string, std::string> out;

	auto quotes = get_quotepairs(header_line);
	decltype(quotes)::const_iterator quotes_it = quotes.begin();
	decltype(quotes)::const_iterator quotes_end = quotes.end();


	next_ch<decltype(quotes)> next_sc(quotes, header_line, ';');
	next_ch<decltype(quotes)> next_col(quotes, header_line, ':');

	std::string::size_type start_pos;

	// capture everything from the right of ':' to the nearest \r\n or ';'
	{
		auto col_pos = next_col.next().get();
		if (col_pos == std::string::npos)
			goto ret;
		for (++col_pos; header_line[col_pos] == ' '; ++col_pos);
-
		auto end_pos = col_pos;
	
		// case 1: quoted-string
		if (header_line[col_pos] == '"') {
			auto qi = std::find_if(quotes.begin(), quotes.end(),
						[&] (decltype(quotes.front()) const& q) {
							return (col_pos == std::get<0>(q));
						});
			++col_pos;
			if (qi != quotes.end()) {
				end_pos = std::get<1>(*qi);
				goto finish_first;
			}
		}
		// case 2: next ';'
		end_pos = next_sc.copy().next().get();
		if (end_pos != std::string::npos) {
			for (--end_pos; header_line[end_pos] == ' '; --end_pos);
			++end_pos;
			goto finish_first;
		}
		// case 3: terminating \r\n
		end_pos = header_line.find("\r\n", start_pos);
		if (end_pos != std::string::npos) {
			for (--end_pos; header_line[end_pos] == ' '; --end_pos);
			++end_pos;
			goto finish_first;
		}
		goto ret;
	finish_first:
		out[""] = header_line.substr(col_pos, end_pos - col_pos);
		start_pos = end_pos + 1;
	}

	// capture (; *$${lc($k)}?(=($v|"$v")))
	// till \r\n
	for (bool last = false; !last; ) {
	
		std::string next_k;
		
		start_pos = next_sc.next().get();
		if (start_pos == std::string::npos)
			goto ret;
		for (++start_pos; header_line[start_pos] == ' '; ++start_pos);
		end_pos = start_pos;
		for (; tokens[widen_cast<int>(header_line[end_pos])] & tok_flags::noquote; ++end_pos);
		auto next = end_pos;
		for (; header_line[next] == ' '; ++next);

		for (char ch : header_line.substr(start_pos, end_pos - start_pos))
			next_k += narrow_cast<char>(lower[widen_cast<int>(ch)]);

		switch (header_line[next]) {
		case ';':
			next_sc.advance_to_pos(next);
			end_pos = start_pos;
			break;
		case '=': 
			for (start_pos = next + 1; header_line[start_pos] == ' '; ++start_pos);
			
			// case 1: quoted-string
			if (header_line[start_pos] == '"') {
				auto qi = std::find_if(quotes.begin(), quotes.end(),
							[&] (decltype(quotes.front()) const& q) {
								return (start_pos == std::get<0>(q));
							});
				++start_pos;
				if (qi != quotes.end()) {
					end_pos = std::get<1>(*qi);
					goto finish_v;
				}
			}
			// case 2: next ';'
			end_pos = next_sc.advance_to_pos(start_pos).copy().next().get();
			if (end_pos != std::string::npos) {
				for (--end_pos; header_line[end_pos] == ' '; --end_pos);
				++end_pos;
				goto finish_v;
			}
			// case 3: terminating \r\n
			end_pos = header_line.find("\r\n", start_pos);
			if (end_pos != std::string::npos) {
				for (--end_pos; header_line[end_pos] == ' '; --end_pos);
				++end_pos;
				last = true;
				goto finish_v;
			}
			goto ret;		
		case '\r':
			last = true;
			break;
		}
	finish_v:
		out[next_k] = header_line.substr(start_pos, end_pos - start_pos);
	}

ret:
	return out;
}

// TODO: LRU for the iconv types
std::wstring rfc2047_decode(std::string const& encoded_word) {
	// =?(charset)?([BQ])?(encoded-text)?=	
	std::string::size_type start_pos = 0;
	for (; encoded_word[start_pos] == ' '; ++start_pos);
	if (encoded_word.substr(start_pos, 2) != "=?")
		goto ret_blank;
	start_pos += 2;
	std::string::size_type next = encoded_word.find('?', start_pos);
	if (next == std::string::npos)
		goto ret_blank;

	std::string encoding = encoded_word.substr(start_pos, next - start_pos);
	
	std::unique_ptr<MOSH_FCGI::Converter> conv(nullptr);
	switch (encoded_word[next + 1]) {
	case 'b':
	case 'B':
		conv.reset(MOSH_FCGI::get_conv("base64"));
		break;
	case 'q':
	case 'Q':
		conv.reset(MOSH_FCGI::get_conv("quoted-printable"));
		break;
	default:
		goto ret_blank;
	}
	next += 2;
	if (next != '?')
		goto ret_blank;
	++next;	
	start_pos = next;
	next = encoded_word.find("?=", start_pos);
	if (next == std::string::npos)
		goto ret_blank;
	const char* dummy;
	
	iconv_t ic = iconv_open("UTF-8", encoding.c_str());

	if (ic == (iconv_t)-1)
		goto ret_blank;
	
	MOSH_FCGI::u_string xbuf(conv->in(encoded_word.data() + start_pos, encoded_word.data() + next, dummy));
	MOSH_FCGI::u_string u8buf;
	u8buf.resize(xbuf.size() * 4 + 1);
	
	const char* in = xbuf.data();
	char* out = const_cast<MOSH_FCGI::u_char*>(u8buf.data());
	size_t in_size = xbuf.size();
	size_t out_size = u8buf.size();

	if (iconv(ic, &const_cast<char*>(in), &in_size, &out, &out_size) == -1) {
		iconv_close(ic);
		goto ret_blank;
	}
	
	iconv_close(ic);
	u8buf.resize(out_size);

	ssize_t u8len = utf8_length(u8buf.data(), u8buf.data() + u8buf.size(), dummy);
	std::wstring ret;
	ret.resize(u8len);

	wchar_t* dummy2;
	int r = utf8_in(u8buf.data(), u8buf.data() + uf8buf.size(), dummy,
			const_cast<wchar_t*>(ret.data()), const_cast<wchar_t*>(ret.data()) + ret.size(), dummy2);
	if (r != 0)
		goto ret_blank;
	return ret;
ret_blank:
	return L"";
}

std::string filtering::filter(std::string const& str, filtering::field f, filtering::subfield_union subf) {
	auto xid = std::make_pair(f,subf);
	filtering_param const& fp = filters.find(xid);
	if (&fp == &(filters.end()))
		return "";
	xassert(fp.id == xid, "mime::filter: ID mismatch");
	return fp.func(str);
}

SRC_END


