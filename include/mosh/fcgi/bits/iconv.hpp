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

#include <string>
#include <locale>
#include <cerrno>
#include <stdexcept>
extern "C" {
#include <iconv.h>
}
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace Iconv {

struct IC_state {
	iconv_t ic_in;
	iconv_t ic_out;
};


/*! @brief Make an iconv state.
 * @param[in] istate external charset
 * @param[in] ostate internal charset
 * @return IC_state with given charsets
 */
IC_state* make_state(const std::string& istate, const std::string& ostate) {
	Iconv::IC_state* ic = new Iconv::IC_state;
	if (ic == nullptr)
		throw std::bad_alloc();
	if (istate == ostate) {
		ic->ic_in = nullptr;
		ic->ic_out = nullptr;
		return ic;
	}
	
	errno = 0;
	ic->ic_in = iconv_open(ostate.c_str(), istate.c_str());
	if (errno != 0)
		goto icm_undo_in;
	ic->ic_out = iconv_open(istate.c_str(), ostate.c_str());
	if (errno != 0)
		goto icm_undo_out;
	return ic;

icm_undo_out:
	iconv_close(ic->ic_in);	
icm_undo_in:
	delete ic;
	throw std::invalid_argument("iconv: cannot convert between " + istate + " and " + ostate);
}

struct Deleter {
	void operator()(IC_state* ic) const {
		iconv_close(ic->ic_in);
		iconv_close(ic->ic_out);
		delete ic;
	}	
};

}

MOSH_FCGI_END

#endif
