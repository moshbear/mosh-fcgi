//! @file bits/boyer-moore.cpp - Boyer-moore string searcher
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
// Copy-pasted from wikipedia.

#include <algorithm>
#include <cstddef>
#include <climits>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <cassert>

#include <mosh/fcgi/bits/boyer_moore.hpp>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

#include <src/u.hpp>
#include <src/namespace.hpp>

namespace {


std::vector<short> compute_prefix_func(const std::vector<SRC::uchar>& p) {
        short k = 0;
	std::vector<short> pi(p.size());

        pi[0] = k;
        for (size_t q = 1; q < p.size(); q++) {
                while ((k > 0) && (p[k] != p[q]))
                        k = pi[k - 1];
                if (p[k] == p[q])
                        k++;
                pi[q] = k;
        }
	return pi;
}

std::vector<int> prepare_badcharacter_heuristic(const std::vector<SRC::uchar>& str) {
	std::vector<int> result(1 << CHAR_BIT, str.size());
	for (size_t i = 0; i < str.size(); i++)
		result[str[i]] = i;
	return result;
}
 
std::vector<int> prepare_goodsuffix_heuristic(const std::vector<SRC::uchar>& normal) {
	std::vector<SRC::uchar> reversed(normal);
	std::reverse(reversed.begin(), reversed.end());
	std::vector<short> prefix_reversed = compute_prefix_func(reversed);

	std::vector<int> result(normal.size() + 1);
 
        /* can't figure out how to handle first and last positions with the rest
        its algorithm is slightly different */
 
        //result of 0 will only be accessed when we find a match
        //so it stores the good suffix skip of the first character
        //(last in reverse calculation)
        result[0] = normal.size() - prefix_reversed[normal.size() - 1];
        //The last value in the prefix calculation is always
        //the same for a string in both directions
	result[normal.size()] = 1;
 	for (size_t i = 1; prefix_reversed[i++]; result[normal.size()]++);
 
        for (size_t i = 1; i < normal.size(); i++) {
                size_t test = 0;
		size_t j = i;
                for (; j < normal.size() - 1; j++) {                  
			assert(prefix_reversed[j] >= 0); // If this fails, send a bug report
                        if (prefix_reversed[j] == i) {
                                test = 1;
                                if (prefix_reversed[j + 1] == 0) {
                                        test = 2;
                                        break;
                                }
                        }
                }
 		int& res = result[normal.size() - i];
                if (test == 1)
                        res = normal.size();
                else if (test == 2)
                        res = j + 1 - i;
                else
                        res = normal.size() - prefix_reversed[normal.size() - 1];
        }
	return result;
}

} // anonymous namespace

MOSH_FCGI_BEGIN
/*
 * Boyer-Moore search algorithm
 */
void Boyer_moore_searcher::arr_init(const uchar* needle, size_t len) {
        if (len != 0) {
		if (needle == 0)
			throw std::invalid_argument("Precondition (needle != 0 || len == 0) failed");
		this->needle = std::vector<uchar>(needle, needle + len);
	        // Initialize heuristics
		badcharacter = prepare_badcharacter_heuristic(this->needle);
		goodsuffix = prepare_goodsuffix_heuristic(this->needle);
	}
	this->needle.push_back('\0');
	this->needle_len = len;
}

ssize_t Boyer_moore_searcher::_search(const uchar* haystack, size_t haystack_len) const {
	if (haystack_len == 0 || needle_len == 0)
		return 0;
	if (needle_len == 0 && haystack_len != 0)
		return -1;
        // Boyer-Moore search 
        ssize_t s = 0;
        while (s <= (haystack_len - needle_len)) {
                size_t j = needle_len;
                while ((j > 0) && (needle[j - 1] == haystack[s + j - 1]))
                        j--;
                if (j > 0) {
                        int k = badcharacter[haystack[s + j - 1]];
                        int m;
                        if (k < static_cast<int>(j) && ((m = j - k - 1) > goodsuffix[j]))
                                s += m;
                        else
                                s += goodsuffix[j];
                } else
			return s;
        }
	return -1;
}

MOSH_FCGI_END
