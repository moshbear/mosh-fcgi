//! @file mosh/fcgi/http/session/session_base.tcc Template implementation for Session_base
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


#ifndef FASTCGIPP_HTTP_SESSION_SESSION_BASE_TCC
#define FASTCGIPP_HTTP_SESSION_SESSION_BASE_TCC

#include <algorithm>
#include <locale>
#include <memory>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <cstring>
#include <mosh/fcgi/http/conv/converter.hpp>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/http/session/funcs.hpp>
#include <mosh/fcgi/http/session/session_base.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

namespace stdph = std::placeholders;

extern template
std::string Session_base<char>::to_unicode(u_string& u);

extern template
u_string Session_base<uchar>::to_unicode(u_string& u);

extern template
std::wstring Session_base<wchar_t>::to_unicode(u_string& u);

// Const versions
extern template
std::string Session_base<char>::to_unicode(u_string const& u);

extern template
u_string Session_base<uchar>::to_unicode(u_string const& u);

extern template
std::wstring Session_base<wchar_t>::to_unicode(u_string const& u);

template<typename ct>
u_string Session_base<ct>::process_encoded_data() {
	u_string ret;
	if (this->conv) {
		const char* f_next;
		ret = std::move(this->conv->in(this->ebuf.data(), this->ebuf.data() + this->ebuf.size(), f_next));
		this->ebuf.erase(0, f_next - this->ebuf.data());
	} else { // null converter; bitwisedly copy
		ret = std::move(std::string(sign_cast<const uchar*>(this->ebuf.data()),
						sign_cast<const uchar*>(this->ebuf.data()) + this->ebuf.size()));
		this->ebuf.clear();
	}
		return ret;
}

// Fill environment
// fill_gets(QUERY_STRING) is called from here
// Init of (ue|mp)_regex_cache too (instead of in ctor)
template <class char_type>
void Session_base<char_type>::fill(const uchar* data, size_t size) {
	this->bbuf.append(data, data + size);
	session::do_fill(this->bbuf, this->envs,
			[&] ()  { return this->ue_init(); },
			[&] (std::string const& a1) { return this->mp_init(a1); },
			[&] (const char* a1, size_t a2) { this->fill_ue_oneshot(a1, a2); },
			[&] (const char* a1, size_t a2) { session::process_cookies(a1, a2, this->cookies, this->cookies_g); }
	);

}

template <class char_type>
void Session_base<char_type>::fill_ue_oneshot(const char* data, size_t size,
	typename Session_base<char_type>::Kv& dest)
{
	// Since this isn't chunked (i.e. it's one-shot), we don't have to worry about partial input.
	// Hence, this function is only half as long as fill_ue(const char*, size_t).
	const char* const data_end = data + size;
	
	while (data != data_end) {
		
		u_string k, v;
		int u_stat = session::process_urlencoded_kv(data, size, k, v);

		// Check if this is a continuation or not.
		if (u_stat < 0)
			return;
		
		T_string name = this->to_unicode(k);
		T_string value = this->to_unicode(v);
		
		if (u_stat > 0) {
			data += u_stat + 1;
			size = data_end - data;
		} else
			data = data_end;
		if (dest.find(name) == dest.end()) {
			Multi_v	e(name, value);
			e.enable_unique_mode();
			dest[name] = std::move(e);
		} else {
			dest[name] << std::move(value);
		}
	}
}

} // namespace http

MOSH_FCGI_END

#endif


