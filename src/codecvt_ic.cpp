//! @file codecvt_ic.cpp Codecvt specialization for Iconv::IC_state* state type
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

#include <locale>
#include <mosh/fcgi/bits/iconv.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

#define DO_IN(i, e, s) \
	template<> \
	codecvt_base::result \
	codecvt<i, e, s>::do_in(s&, const e*, const e*, const e*&, i*, i*, i*&) const { \
		return codecvt_base::noconv; \
	}

#define DO_OUT(i, e, s) \
	template<> \
	codecvt_base::result \
	codecvt<i, e, s>::do_out(s&, const i*, const i*, const i*&, e*, e*, e*&) const { \
		return codecvt_base::noconv; \
	}

#define DO_UNSHIFT(i, e, s) \
	template<> \
	codecvt_base::result \
	codecvt<i, e, s>::do_unshift(s&, e*, e*, e*&) const { \
		return codecvt_base::noconv; \
	}

#define DO_ENCODING(i,e, s) \
	template<> int codecvt<i, e, s>::do_encoding() const throw() { return 0; }

#define DO_ALWAYS_NOCONV(i, e, s) \
	template<> bool codecvt<i, e, s>::do_always_noconv() const throw() { return true; }

#define DO_LENGTH(i, e, s) \
	template<> int codecvt<i, e, s>::do_length(s&, const e*, const e*, size_t) const { return -1; }

#define DO_MAX_LENGTH(i, e, s) \
	template<> int codecvt<i, e, s>::do_max_length() const throw() { return 0; }

#define IC MOSH_FCGI::Iconv::IC_state*

namespace std {

DO_IN(char, char, IC)
DO_OUT(char, char, IC)
DO_UNSHIFT(char, char, IC)
DO_ENCODING(char, char, IC)
DO_ALWAYS_NOCONV(char, char, IC)
DO_LENGTH(char, char, IC)
DO_MAX_LENGTH(char, char, IC)

DO_IN(wchar_t, char, IC)
DO_OUT(wchar_t, char, IC)
DO_UNSHIFT(wchar_t, char, IC)
DO_ENCODING(wchar_t, char, IC)
DO_ALWAYS_NOCONV(wchar_t, char, IC)
DO_LENGTH(wchar_t, char, IC)
DO_MAX_LENGTH(wchar_t, char, IC)

}

