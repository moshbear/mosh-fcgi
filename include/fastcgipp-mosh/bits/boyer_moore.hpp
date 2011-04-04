//! \file boyer_moore.hpp - Boyer-moore string search
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
// modification of wikipedia example

#ifndef BOYER_MOORE_HPP
#define BOYER_MOORE_HPP

#include <boost/shared_array.hpp>
#include <string>
#include <cstring>
#include <climits>

class Boyer_moore_searcher {
	boost::shared_array<int> badcharacter;
	boost::shared_array<int> goodsuffix;
	boost::shared_array<char> needle;
	size_t needle_len;
	typedef const char* cptr;
public:
	Boyer_moore_searcher() : needle_len(0) {
	}

	// Throws std::invalid_argument if precondition (str != 0 || len == 0) is unsatisfied
	Boyer_moore_searcher(const char* str, size_t len) {
		__init__(str, len);
	}
	Boyer_moore_searcher(const std::string& s) {
		__init__(s.data(), s.size());
	}

	inline const char* search(const char* haystack) const {
		return search(haystack, strlen(haystack));
	}
	inline const char* search(const char* haystack, size_t haystack_len) const {
		ssize_t res = _search(haystack, haystack_len);
		if (res == -1)
			return 0;
		return haystack + res;
	}
	inline bool search(const char* haystack, cptr& res) const {
		return search(haystack, strlen(haystack), res);
	}
	bool search(const char* haystack, size_t haystack_len, cptr& res) const {
		res = search(haystack, haystack_len);
		return found_match(haystack, haystack_len, res);
	}



	static bool found_match(const char* haystack, size_t haystack_len, const char* result) {
		return ((result != 0) && (result >= haystack) && (result < (haystack + haystack_len)));
	}

private:
	void __init__(const char* str, size_t len);
	ssize_t _search(const char* haystack, size_t haystack_len) const;
};

#endif
		
