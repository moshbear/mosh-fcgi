//! @file mosh/fcgi/bits/cmp.hpp Templated comparators
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

#ifndef MOSH_FCGI_CMP_HPP
#define MOSH_FCGI_CMP_HPP

#include <algorithm>
#include <utility>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! Type of comparison to perform
enum class Cmp_test {
	eq, //!< Compare for equality
	ne, //!< Compare for inequality
	lt, //!< Compare for less-than
	le, //!< Compare for less-than-or-equal
	ge, //!< Compare for greater-than-or-equal
	gt //!< Compare for greater-than
};

/*! @brief Compare two objects
 * @param o1 The first object
 * @param o2 The second object
 * @param test The type of test to perform
 * @return The result of test(o1, o2)
 */
template <typename T>
bool cmp(T&& o1, T&& o2, Cmp_test test) {
	switch (test) {
	case Cmp_test::eq: return (o1 == o2);
	case Cmp_test::ne: return (o1 != o2);
	case Cmp_test::lt: return (o1 < o2);
	case Cmp_test::le: return (o1 <= o2);
	case Cmp_test::ge: return (o1 >= o2);
	case Cmp_test::gt: return (o1 > o2);
	}
	return false;
}

//! @brief Specialization for @c booleans
template<> bool cmp(const bool& o1, const bool& o2, Cmp_test test) {
	return cmp(o1 ? 1 : 0, o2 ? 1 : 0, test);
}
		

/*! @brief Compare two object ranges
 * Compares two object ranges defined by [_1_Begin, _1_End), [_2_Begin, _2_End) and tests their values.
 * @param _1_Begin Start of first range
 * @param _1_End End of first range
 * @param _2_Begin Start of second range
 * @param _2_End End of second range
 * @param test The type of test to perform
 * @retval @c false if cmp(..., test) fails anywhere within the smaller range
 * @retval @c true if cmp(..., test) passes on iterator distance
 * @retval @c false otherwise
 */
template <class InputIterator>
bool range_cmp(InputIterator _1_Begin, InputIterator _1_End, InputIterator _2_Begin, InputIterator _2_End, Cmp_test test) {
	while (_1_Begin != _1_End && _2_Begin != _2_End) {
		if (!cmp(*_1_Begin, *_2_Begin, test))
			return false;
		++_1_Begin;
		++_2_Begin;
	}
	return cmp(std::distance(_1_Begin, _1_End), std::distance(_2_Begin, _2_End), test);
}

/*! @brief Compare two object ranges (custom comparator)
 * Compares two object ranges defined by [_1_Begin, _1_End), [_2_Begin, _2_End) and tests their values.
 * @param _1_Begin Start of first range
 * @param _1_End End of first range
 * @param _2_Begin Start of second range
 * @param _2_End End of second range
 * @param test The type of test to perform
 * @param comp The comparator to use; must contain public bool operator()(T const&, T const&, Cmp_test)
 * @retval @c false if cmp(..., test) fails anywhere within the smaller range
 * @retval @c true if cmp(..., test) passes on iterator distance
 * @retval @c false otherwise
 */
template <class InputIterator, class My_cmp>
bool range_cmp(InputIterator _1_Begin, InputIterator _1_End, InputIterator _2_Begin, InputIterator _2_End, Cmp_test test, My_cmp comp) {
	while (_1_Begin != _1_End && _2_Begin != _2_End) {
		if (!comp(*_1_Begin, *_2_Begin, test))
			return false;
		++_1_Begin;
		++_2_Begin;
	}
	return cmp(_1_Begin != _1_End, _2_Begin != _2_End, test);
}

/*! @brief Compare two iterable objects
 * Runs range_cmp on (_1.begin(), _1.end(), _2.begin(), _2.end(), test).
 * @param _1 First iterable object
 * @param _2 Second iterable object
 * @param test The type of test to perform
 * @return the value of range_cmp(_1.begin(), _1.end(), _2.begin(), _2.end(), test)
 */
template <class Iterable>
bool range_cmp(const Iterable& _1, const Iterable& _2, Cmp_test test) {
	return range_cmp(_1.begin(), _1.end(), _2.begin(), _2.end(), test);
}

/*! @brief Compare two iterable objects (custom comparator)
 * Runs range_cmp on (_1.begin(), _1.end(), _2.begin(), _2.end(), test, comp).
 * @param _1 First iterable object
 * @param _2 Second iterable object
 * @param test The type of test to perform
 * @param comp The comparator to use; must contain public bool operator()(T const&, T const&, Cmp_test)
 * @return the value of range_cmp(_1.begin(), _1.end(), _2.begin(), _2.end(), test, comp)
 */
template <class Iterable, class My_cmp>
bool range_cmp(const Iterable& _1, const Iterable& _2, Cmp_test test, My_cmp comp) {
	return range_cmp(_1.begin(), _1.end(), _2.begin(), _2.end(), test, comp);
}

MOSH_FCGI_END

#endif


