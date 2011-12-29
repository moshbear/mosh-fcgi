//! @file  mosh/fcgi/bits/popcount.hpp Population count
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

#ifndef MOSH_FCGI_POPCOUNT_HPP
#define MOSH_FCGI_POPCOUNT_HPP

#include <cstdint>
#include <limits>
#include <type_traits>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, size_t>::type
popcount(T x) {
	size_t pop = 0;
	for (; x != 0; ++pop) {
		x &= x - 1;
	}
	return pop;
}

MOSH_FCGI_END

#endif
