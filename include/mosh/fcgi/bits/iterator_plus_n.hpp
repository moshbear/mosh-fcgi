//! @file  mosh/fcgi/bits/iterator_plus_n.hpp iterator_plus_n template function
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

#ifndef MOSH_FCGI_ITERATOR_PLUS_N_HPP
#define MOSH_FCGI_ITERATOR_PLUS_N_HPP

#include <algorithm>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @brief Safe iterator increment
 * 
 * This function checks if @c begin + @c n is inside [@c begin, @c end].
 * If not, it returns @c end.
 *
 * T(d, n) is O(1) for @c RandomAccessIterators, and O(d + n) otherwise/
 *
 * @param begin Iterator to start
 * @param end Iterator to end; if @c begin + @c n goes past here, return this value.
 * @param n Distance to increment
 * @return @c begin + @c n if @c begin + @c n &lt;= @c end; @c end otherwise
 */
template <typename Iterator>
Iterator iterator_plus_n(Iterator begin, Iterator end, size_t n) {
	size_t max_n = std::distance(begin, end);
	if (n > max_n) {
		return end;
	} else {
		std::advance(begin, n);
		return begin;
	}
}

MOSH_FCGI_END

#endif


