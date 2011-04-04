//! \file boyer-moore - Boyer-moore string searcher
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
// Copy-pasted from wikipedia.

#include <cstddef>
#include <climits>
#include <cstring>
#include <stdexcept>

#include <boost/scoped_array.hpp>
#include <fastcgipp-mosh/bits/boyer_moore.hpp>

typedef char char_t;

namespace { // local static functions


// Preconditions:
// 	p != 0
// 	(sizeof pi / sizeof pi[0]) == len
void compute_prefix_func(const char *p, size_t len, short *pi)
{
        short k = 0;
        pi[0] = k;

        for (size_t q = 1; q < len; q++) {
                while ((k > 0) && (p[k] != p[q]))
                        k = pi[k - 1];
                if (p[k] == p[q])
                        k++;
                pi[q] = k;
        }
}
 
// Preconditions: 
// 	* str != 0
// 	* (sizeof result / sizeof result[0]) == 1 << (CHAR_BIT - 1)
void prepare_badcharacter_heuristic(const char *str, size_t size, int *result)
{
	for (size_t i = 0; i < (1 << CHAR_BIT); ++i)
		result[i] = size;

	for (size_t i = 0; i < size; i++)
		result[static_cast<size_t>(str[i])] = i;
}
 
// Preconditions:
// 	normal != 0
void prepare_goodsuffix_heuristic(const char *normal, size_t size, int *result)
{
        const char *left = normal;
        const char *right = left + size;
        char reversed[size + 1];
        char *tmp = reversed + size;
        size_t i;
	size_t j;
        char test;
 
        /* reverse string */
        *tmp = 0;
        while (left < right)
                *(--tmp) = *(left++);


	boost::scoped_array<short> prefix_reversed(new short[size]);
        compute_prefix_func(reversed, size, prefix_reversed.get());
 
        /* can't figure out how to handle first and last positions with the rest
        its algorithm is slightly different */
 
        //result of 0 will only be accessed when we find a match
        //so it stores the good suffix skip of the first character
        //(last in reverse calculation)
        result[0] = size - prefix_reversed[size - 1];
        //The last value in the prefix calculation is always
        //the same for a string in both directions
 	for (i = 1, result[size] = 1; prefix_reversed[i++]; result[size]++);
 
        for (i = 1; i < size; i++) {
                test = 0;
                for (j = i; j < size - 1; j++) {                   
                        if (prefix_reversed[j] == i) {
                                test = 1;
                                if (prefix_reversed[j + 1] == 0) {
                                        test = 2;
                                        break;
                                }
                        }
                }
 
                if (test == 1)
                        result[size - i] = size;
                else if (test == 2)
                        result[size - i] = j + 1 - i;
                else
                        result[size - i] = size - prefix_reversed[size - 1];
        }
}
} // anonymous namespace

/*
 * Boyer-Moore search algorithm
 */
void Boyer_moore_searcher::__init__(const char *needle, size_t len) {
        if(len != 0) {
		if (needle == 0)
			throw std::invalid_argument("Precondition (needle != 0 || len == 0) failed");
	        // Initialize heuristics
		this->badcharacter.reset(new int[1 << CHAR_BIT]);
		this->goodsuffix.reset(new int[len + 1]);
	        prepare_badcharacter_heuristic(needle, len, this->badcharacter.get());
		prepare_goodsuffix_heuristic(needle, len, &(*(this->goodsuffix.get())));
		this->needle.reset(new char[len + 1]);
		memcpy(this->needle.get(), needle, len + 1);
	}
	this->needle_len = len;
}

ssize_t Boyer_moore_searcher::_search(const char *haystack, size_t haystack_len) const {
	
	if (haystack_len == 0 || needle_len == 0)
		return 0;
	if (needle_len == 0 && haystack_len != 0)
		return -1;
        // Boyer-Moore search 
        size_t s = 0;
        while(s <= (haystack_len - needle_len))
        {
                size_t j = needle_len;
                while((j > 0) && (needle[j - 1] == haystack[s + j - 1]))
                        j--;
 
                if(j > 0) {
                        int k = badcharacter[(size_t) haystack[s + j - 1]];
                        int m;
                        if(k < (int)j && ((m = j - k - 1) > goodsuffix[j]))
                                s += m;
                        else
                                s += goodsuffix[j];
                } else {
			return s;
                }
        }
	return -1;
}
