//! \file assign-or-append.hpp - Append if pred is true; assign otherwise
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
#ifndef FASTCGIPP_AOA
#define FASTCGIPP_AOA

namespace Fastcgipp_m0sh {

template <typename T>
inline T& append_or_assign(T* obj, const T& app,bool pred) {
	if (pred)
		(*obj) += app;
	else
		(*obj) = app;
	return *obj;
}

}

#endif


