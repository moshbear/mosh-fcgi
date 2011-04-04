//! \file url.cpp - url decoding stuff
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


#include <string>
#include <algorithm>
#include <cctype>
#include <fastcgipp-mosh/unicode.hpp>

namespace Fastcgipp_m0sh
{


namespace Http
{

namespace Url {
	/* Preconditions: first, second ~ [0-9A-Fa-f]
	 *
	 */
	char hexUnescape(char first, char second) {	
		int __first = first & 0x0F;
		int __second = second & 0x0F;
		if (first & 0x40) {
			__first += 9;
		}
		if (second & 0x40)
			__second += 9;
     		__first <<= 4;
		return static_cast<char>(__first | __second);
	}
}

}
}



