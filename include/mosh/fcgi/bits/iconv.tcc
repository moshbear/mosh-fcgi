//! @file mosh/fcgi/bits/iconv.tcc Iconv codecvt facet
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

#ifndef MOSH_FCGI_ICONV_TCC
#define MOSH_FCGI_ICONV_TCC

#include <algorithm>
#include <locale>
#include <cwchar>
#include <cerrno>
#include <stdexcept>
extern "C" {
#include <iconv.h>
}
#include <mosh/fcgi/bits/iconv.hpp>
#include <mosh/fcgi/bits/iconv_cvt.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

// Iconv state

struct Iconv::IC_state {
	iconv_t ic_in;
	iconv_t ic_out;
};

Iconv::IC_state* Iconv::make_state(const std::string& istate, const std::string& ostate)
{
	Iconv::IC_state* ic = new Iconv::IC_state;
	if (ic == nullptr)
		throw std::bad_alloc("iconv: no memory for state");
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

void Iconv::Deleter::operator()(Iconv::IC_state* ic) const {
	iconv_close(ic->ic_in);
	iconv_close(ic->ic_out);
	delete ic;
}

// Iconv cvt

template <typename in_type, typename ex_type>
std::codecvt_base::result Iconv_cvt<in_type, ex_type>::do_in(
			Iconv::IC_state*& ic, const ex_type* from,
			const ex_type* from_end, const ex_type*& from_next,
			in_type* to, in_type* to_end, in_type*& to_next
		) const
{
	const char* ic_from = reinterpet_cast<const char*>(from);
	char* ic_to = reinterpret_cast<char*>(to);
	size_t ic_fsize = (from_end - from) * sizeof(ex_type);
	size_t ic_tsize = (to_end - to) * sizeof(in_type);
	
	if (ic->ic_in == nullptr) {
		size_t mincopy = std::min(ic_fsize, ic_tsize);
		memcpy(ic_to, ic_from, mincopy);
		from_next = from + mincopy;
		to_next = to + mincopy;
		return std::codecvt_base::noconv;
	}
	
	errno = 0;
	size_t ic_ret = iconv(ic->ic_in, &ic_from, &ic_fsize, &ic_to, &ic_tsize);

	std::codecvt_base::result cvt_ret = std::codecvt_base::ok;
	if (ic_ret == -1) {
		switch (errno) {
		case E2BIG: // Incomplete input
		case EINVAL: // Incomplete output
			cvt_ret = std::codecvt_base::partial;		
			break;
		case EILSEQ: // Invalid input
			cvt_ret = std::codecvt_base::error;
			break;
	}
	
	from_next = from_end - ((ic_fsize / sizeof(ex_type)) + ((ic_fsize % sizeof(ex_type)) ? 1 : 0);
	to_next = to_end - ((ic_tsize / sizeof(in_type)) + ((ic_tsize % sizeof(in_type)) ? 1 : 0);
	return cvt_ret;
}

template <typename in_type, typename ex_type>
std::codecvt_base::result Iconv_cvt<in_type, ex_type>::do_out(
			Iconv::IC_state*& ic, const in_type* from,
			const in_type* from_end, const in_type*& from_next,
			ex_type* to, ex_type* to_end, ex_type*& to_next
		) const
{
	const char* ic_from = reinterpet_cast<const char*>(from);
	char* ic_to = reinterpret_cast<char*>(to);
	size_t ic_fsize = (from_end - from) * sizeof(ex_type);
	size_t ic_tsize = (to_end - to) * sizeof(in_type);
	
	if (ic->ic_out == nullptr) {
		size_t mincopy = std::min(ic_fsize, ic_tsize);
		memcpy(ic_to, ic_from, mincopy);
		from_next = from + mincopy;
		to_next = to + mincopy;
		return std::codecvt_base::noconv;
	}
	
	errno = 0;
	size_t ic_ret = iconv(ic->ic_out, &ic_from, &ic_fsize, &ic_to, &ic_tsize);

	std::codecvt_base::result cvt_ret = std::codecvt_base::ok;
	if (ic_ret == -1) {
		switch (errno) {
		case E2BIG: // Incomplete input
		case EINVAL: // Incomplete output
			cvt_ret = std::codecvt_base::partial;		
			break;
		case EILSEQ: // Invalid input
			cvt_ret = std::codecvt_base::error;
			break;
	}
	
	from_next = from_end - ((ic_fsize / sizeof(ex_type)) + ((ic_fsize % sizeof(ex_type)) ? 1 : 0);
	to_next = to_end - ((ic_tsize / sizeof(in_type)) + ((ic_tsize % sizeof(in_type)) ? 1 : 0);
	return cvt_ret;
}

MOSH_FCGI_END

#endif
