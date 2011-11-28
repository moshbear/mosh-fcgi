//! @file mosh/fcgi/bits/iconv.hpp Iconv facet miscellany
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

#ifndef MOSH_FCGI_ICONV_HPP
#define MOSH_FCGI_ICONV_HPP

#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace Iconv {

struct IC_state;

/*! @brief Make an iconv state.
 * @param[in] istate external charset
 * @param[in] ostate internal charset
 * @return IC_state with given charsets
 */
IC_state* make_state(const std::string& istate, const std::string& ostate);

struct Deleter {
	void operator()(IC_state*) const;
};

}

MOSH_FCGI_END

#endif
