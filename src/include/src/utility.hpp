//! @file  src/utility.hpp Miscellaneous utilities
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

#ifndef SRC_UTILITY_HPP
#define SRC_UTILITY_HPP

#include <tuple>
#include <type_traits>

#include <src/namespace.hpp>

SRC_BEGIN

/*! @brief Widening cast from one type to another.
 *
 * Widens a value of type %From to type %To and returns it.
 *
 * @note Requires that %From be convertible to %To
 *
 * @tparam To destination type
 * @tparam From source type
 * @param expr value to widen
 */
template <typename To, typename From>
typename std::enable_if<std::is_convertible<typename std::decay<From>::type, To>, To>::type
widen_cast(From expr) {
	To t{expr};
	return t;
}

/*! @brief Narrowing cast from one type to another.
 *
 * Narrows a value of type %From to type %To and returns it.
 *
 * @note Requires that %From be convertible to %To
 *
 * @tparam To destination type
 * @tparam From source type
 * @param expr value to widen
 */
template <typename To, typename From>
typename std::enable_if<std::is_convertible<typename std::decay<From>::type, To>, To>::type
narrow_cast(From expr) {
	To t(expr);
	return t;
}

/*! @brief Tuple type helper for repetitions of types.
 *
 * @tparam N number of repetitions
 * @tparam T type
 */
template <size_t N, typename T>
struct tuple_rep {
	//! A typedef for a @c std::tuple consisting of N repetitions of T
	typedef decltype(std::tuple_cat(typename tuple_rep<N - 1, T>::type(), std::tuple<T>())) type;
};

template <typename T>
struct tuple_rep<0, T> {
	// Base case: 0 repetitions
	typedef std::tuple<> type;
};

template <typename... Types>
struct common_enum {
	typedef typename std::common_type<typename std::underlying_type<Types>::type ...>::type type;
};

SRC_END

#endif

