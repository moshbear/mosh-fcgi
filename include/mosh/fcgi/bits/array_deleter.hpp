//! @file mosh/fcgi/bits/array_deleter.hpp Array deleter (for std::shared_ptr, unique_ptr)
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

#ifndef MOSH_FCGI_ARRAY_DELETER_HPP
#define MOSH_FCGI_ARRAY_DELETER_HPP

#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! A %Deleter for T[]
template <typename T>
struct Array_deleter {
	void operator () (T* t) const {
		delete[] t;
	}
};

MOSH_FCGI_END

#endif
