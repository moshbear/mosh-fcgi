//! @file  mosh/fcgi/boyer_moore.hpp Boyer-moore string search
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
// modification of wikipedia example

#ifndef MOSH_FCGI_BOYER_MOORE_HPP
#define MOSH_FCGI_BOYER_MOORE_HPP

#include <string>
#include <cstring>
#include <climits>
#include <vector>
#include <mosh/fcgi/bits/types.hpp>

#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! A boyer-moore string searcher
class Boyer_moore_searcher {
	//! Shift-amount array 1
	std::vector<int> badcharacter;
	//! Shift-amount array 2
	std::vector<int> goodsuffix;
	//! String to be searched for
	std::vector<uchar> needle;
	//! Length of string
	size_t needle_len;
public:
	//! Empty initialization
	Boyer_moore_searcher() : needle_len(0) {
	}

	/*! @brief Create a Boyer-moore string searcher from a string
	 * @pre (@c str != 0 || @c len == 0)
	 * @param[in] str pointer to search string
	 * @param[in] len length of search string
	 */
	Boyer_moore_searcher(const uchar* str, size_t len) {
		__init__(str, len);
	}
	/*! @brief Create a Boyer-moore string searcher from a string
	 * @pre (@c str != 0 || @c len == 0)
	 * @param[in] str pointer to search string
	 * @param[in] len length of search string
	 */
	Boyer_moore_searcher(const char* str, size_t len) {
		__init__(reinterpret_cast<const uchar*>(str), len);
	}
	/*! @brief Create a Boyer-moore string searcher from a string
	 * @param[in] s search string
	 */
	Boyer_moore_searcher(const std::basic_string<uchar>& s) {
		__init__(s.data(), s.size());
	}
	/*! @brief Create a Boyer-moore string searcher from a string
	 * @param[in] s search string
	 */
	Boyer_moore_searcher(const std::string& s) {
		__init__(reinterpret_cast<const uchar*>(s.data()), s.size());
	}
	
	Boyer_moore_searcher(const Boyer_moore_searcher& bm)
	: 	badcharacter(bm.badcharacter), goodsuffix(bm.goodsuffix),
		needle(bm.needle), needle_len(bm.needle_len)
	{ }
	
	Boyer_moore_searcher(Boyer_moore_searcher&& bm)
	: 	badcharacter(std::move(bm.badcharacter)), goodsuffix(std::move(bm.goodsuffix)),
		needle(std::move(bm.needle)), needle_len(bm.needle_len)
	{ }

	void swap(Boyer_moore_searcher& bm) {
		badcharacter.swap(bm.badcharacter);
		goodsuffix.swap(bm.goodsuffix);
		needle.swap(bm.needle);
		needle_len = bm.needle_len;
	}

	/*! @brief Search a string for the search string.
	 * @pre haystack is null-terminated
	 * @param[in] haystack string to search
	 * @return pointer to first match or 0 if search string was not found
	 */
	inline const uchar* search(const uchar* haystack) const {
		return search(haystack, strlen(reinterpret_cast<const char*>(haystack)));
	}
	/*! @brief Search a string for the search string.
	 * @pre haystack is null-terminated
	 * @param[in] haystack string to search
	 * @return pointer to first match or 0 if search string was not found
	 */
	inline const char* search(const char* haystack) const {
		return reinterpret_cast<const char*>(search(reinterpret_cast<const uchar*>(haystack)));
	}
	/*! @brief Search a string for the search string.
	 * @param[in] haystack string to search
	 * @param[in] haystack_len length of string to search
	 * @return pointer to first match or 0 if search string was not found
	 */
	const uchar* search(const uchar* haystack, size_t haystack_len) const {
		ssize_t res = _search(haystack, haystack_len);
		if (res == -1)
			return 0;
		return haystack + res;
	}
	/*! @brief Search a string for the search string.
	 * @param[in] haystack string to search
	 * @param[in] haystack_len length of string to search
	 * @return pointer to first match or 0 if search string was not found
	 */
	inline const char* search(const char* haystack, size_t haystack_len) const {
		return search(reinterpret_cast<const uchar*>(haystack), haystack_len);
	}
	/*! @brief Search a string for the search string.
	 * @pre haystack is null-terminated
	 * @param[in] haystack string to search
	 * @param[out] res pointer to first match
	 * @retval @c true if match was found
	 * @retval @c false if match was not found
	 */
	inline bool search(const uchar* haystack, const uchar*& res) const {
		return search(haystack, strlen(reinterpret_cast<const char*>(haystack)), res);
	}
	/*! @brief Search a string for the search string.
	 * @pre haystack is null-terminated
	 * @param[in] haystack string to search
	 * @param[out] res pointer to first match
	 * @retval @c true if match was found
	 * @retval @c false if match was not found
	 */
	inline bool search(const char* haystack, const char*& res) const {
		return search(haystack, strlen(haystack), res);
	}
	/*! @brief Search a string for the search string.
	 * @param[in] haystack string to search
	 * @param[in] haystack_len length of string to search
	 * @param[out] res pointer to first match
	 * @retval @c true if match was found
	 * @retval @c false if match was not found
	 */
	bool search(const uchar* haystack, size_t haystack_len, const uchar*& res) const {
		res = search(haystack, haystack_len);
		return found_match(haystack, haystack_len, res);
	}
	/*! @brief Search a string for the search string.
	 * @param[in] haystack string to search
	 * @param[in] haystack_len length of string to search
	 * @param[out] res pointer to first match
	 * @retval @c true if match was found
	 * @retval @c false if match was not found
	 */
	bool search(const char* haystack, size_t haystack_len, const char*& res) const {
		res = search(haystack, haystack_len);
		return found_match(haystack, haystack_len, res);
	}

protected:
	/*! @brief Return wheter a match was found
	 * If the result is within [haystack, haystack_len), return @c true.
	 * @param[in] haystack Search string
	 * @param[in] haystack_len Length of search string
	 * @param[in] result Result of search(const char*, size_t).
	 * @retval @c true if result indicates a match was found
	 * @retval @c false otherwise
	 */
	static bool found_match(const uchar* haystack, size_t haystack_len, const uchar* result) {
		return ((result != 0) && (result >= haystack) && (result < (haystack + haystack_len)));
	}
	/*! @brief Return wheter a match was found
	 * If the result is within [haystack, haystack_len), return @c true.
	 * @param[in] haystack Search string
	 * @param[in] haystack_len Length of search string
	 * @param[in] result Result of search(const char*, size_t).
	 * @retval @c true if result indicates a match was found
	 * @retval @c false otherwise
	 */
	static bool found_match(const char* haystack, size_t haystack_len, const char* result) {
		return ((result != 0) && (result >= haystack) && (result < (haystack + haystack_len)));
	}
	/*! @brief Search a string for the search string.
	 * @param[in] haystack string to search
	 * @param[in] haystack_len length of string to search
	 * @return position of first match, or -1 if none found
	 */
	ssize_t _search(const uchar* haystack, size_t haystack_len) const;
private:
	/*! @brief Initialize the shift arrays.
	 * @pre (@c str != 0 || @c len == 0)
	 * @param[in] str pointer to search string
	 * @param[in] len length of search string
	 */
	void __init__(const uchar* str, size_t len);
};

MOSH_FCGI_END

#endif
