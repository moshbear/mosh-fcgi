//! @file mosh/fcgi/bits/iconv_gs.hpp Iconv getter-setter
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

#ifndef MOSH_FCGI_ICONV_GS_HPP
#define MOSH_FCGI_ICONV_GS_HPP

#include <string>
#include <locale>
#include <cerrno>
#include <stdexcept>
extern "C" {
#include <iconv.h>
}

#include <mosh/fcgi/bits/native_utf.hpp>
#include <mosh/fcgi/bits/gs.hpp>
#include <mosh/fcgi/bits/iconv.hpp>

#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! Iconv-related classes and functions
namespace Iconv {
//! Helper for _s_ic_state::operator =
IC_state* make_state_helper(const std::string& s) {
	if (s.empty()) {
		if (sizeof(_char_type) > 1)
			return Iconv::make_state("UTF-8", native_utf<sizeof(_char_type)>::value());
		else
			return Iconv::make_state("US-ASCII", native_utf<sizeof(_char_type)>::value());
	} else
		return Iconv::make_state(s, native_utf<sizeof(_char_type)>::value());
}

MOSH_FCGI_SETTER_SMARTPTR_T(std::string, std::shared_ptr<Iconv::IC_state>, make_state_helper, ic_state);

}

MOSH_FCGI_END

#endif
