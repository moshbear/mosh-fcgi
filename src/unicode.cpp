//! \file unicode.cpp - utf8 encode/decode
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
/* unicode.cpp - UTF-8 encoding/decoding
 * 
 * Copyright (C) 2011 Andrey Vul
 *
 */

#include <string>
#include <sstream>
#include <stdexcept>

#include <fastcgipp-mosh/unicode.hpp>

#include <fastcgipp-mosh/bits/utf8_cvt.hpp>

template<> std::wstring Fastcgipp_m0sh::Unicode::utfIn<wchar_t>(const std::string&  s) {
	const size_t bufferSize = 512;
	wchar_t buffer[bufferSize];
	size_t s_size = s.size();
	const char* s_ptr = &(*s.begin());
	using namespace std;

	wstring ret;
	if (s_size) {
		codecvt_base::result cr = codecvt_base::partial;
		while (cr == codecvt_base::partial) {
			wchar_t* it;
			const char* tmpData;
			mbstate_t conversionState = mbstate_t();
			cr = use_facet<codecvt<wchar_t, char, mbstate_t> >(locale(locale::classic(), new Utf8_cvt))
				.in(conversionState, s_ptr, s_ptr + s_size, tmpData, buffer, buffer + bufferSize, it);
			ret.append(buffer, it);
			s_size -= tmpData - s_ptr;
			s_ptr = tmpData;
		}
		if (cr == codecvt_base::error)
			throw runtime_error("codecvt_base::error");
		return ret;
	}
	return wstring();
}

template<> std::string Fastcgipp_m0sh::Unicode::utfOut<wchar_t>(const std::wstring& ws) {
	const size_t bufferSize = 512;
	char buffer[bufferSize];
	size_t ws_size = ws.size();
	const wchar_t* ws_ptr = &(*ws.begin());
	using namespace std;
	string ret;
	if (ws_size) {
		codecvt_base::result cr = codecvt_base::partial;
		while (cr == codecvt_base::partial) {
			char* it;
			const wchar_t* tmpData;
			mbstate_t conversionState = mbstate_t();
			cr = use_facet<codecvt<wchar_t, char, mbstate_t> >(locale(locale::classic(), new Utf8_cvt))
				.out(conversionState, ws_ptr, ws_ptr + ws_size, tmpData, buffer, buffer + bufferSize, it);
			ret.append(buffer, it);
			ws_size -= tmpData - ws_ptr;
			ws_ptr = tmpData;
		}
		if (cr == codecvt_base::error)
			throw runtime_error("codecvt_base::error");
		return ret;
	}
	return string();
}
