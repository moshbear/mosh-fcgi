//! @file  mosh/fcgi/bits/iterator_range_check.hpp - Iterator range check
/***************************************************************************
* Copyright (C) 2012 m0shbear                                              *
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

#ifndef MOSH_FCGI_ITERATOR_RANGE_CHECK_HPP
#define MOSH_FCGI_ITERATOR_RANGE_CHECK_HPP

#include <algorithm>
#include <mosh/fcgi/bits/iterator_plus_n.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @brief Iterator range check
 *
 * Checks if begin + n is refers to an object inside [begin, end].
 *
 * @sa iterator_plus_n
 * @param begin Iterator to start
 * @param end Iterator to end
 * @param n Distance to check
 * @retval true if [begin + n] is inside [begin, end]
 * @retval false otherwise
 */
template <typename Iterator>
bool iterator_range_check(Iterator begin, Iterator end, size_t n) {
	Iterator plus_n = iterator_plus_n(begin, end, n);
	std::advance(begin, n);
	return (plus_n == begin);
}

MOSH_FCGI_END

#endif


