//! \file mosh/fcgi/http/session/session_base.tcc Template implementation for Session_base
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


#ifndef FASTCGIPP_HTTP_SESSION_SESSION_BASE_TCC
#define FASTCGIPP_HTTP_SESSION_SESSION_BASE_TCC

#include <algorithm>
#include <locale>
#include <memory>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <boost/xpressive/xpressive.hpp>
#include <mosh/fcgi/boyer_moore.hpp>
#include <mosh/fcgi/protocol/funcs.hpp>
#include <mosh/fcgi/http/conv/converter.hpp>
#include <mosh/fcgi/bits/singleton.hpp>
#include <mosh/fcgi/bits/iconv.hpp>
#include <mosh/fcgi/bits/iconv_cvt.hpp>
#include <mosh/fcgi/bits/native_utf.hpp>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/http/session/session_base.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

namespace xpr = boost::xpressive;
template <typename ct>
struct Session_base<ct>::regex_cache {
	xpr::sregex boundary;
	regex_cache() :
		boundary("boundary=" >> !xpr::as_xpr('"') >> (xpr::s1 = +(~(xpr::set = '"', ';', ','))) >> !xpr::as_xpr('"'))
	{ }
};

template <typename ct>
void Session_base<ct>::set_charset(const std::string& s) {
	if (s.empty()) {
		if (sizeof(ct) > 1)
			ic.reset(Iconv::make_state("UTF-8", native_utf<sizeof(ct)>::value()));
		else
			ic.reset(Iconv::make_state("US-ASCII", native_utf<sizeof(ct)>::value())); 
	} else
		ic.reset(Iconv::make_state(s, native_utf<sizeof(ct)>::value()));
}

template <>
std::string Session_base<char>::to_unicode() {
	std::string _s = std::move(ubuf);
	return _s;
}

template <>
std::wstring Session_base<wchar_t>::to_unicode() {
	using namespace std;
	wstring ret;
	wchar_t* dummy;
	ret.resize(ubuf.size());
	const char* f_next;
	Iconv::IC_state* i = ic.get();
	use_facet<codecvt<wchar_t, char, Iconv::IC_state*>>(locale(locale::classic(), new Iconv_cvt<wchar_t, char>))
		.in(i, ubuf.data(), ubuf.data() + ubuf.size(), f_next, &(*ret.begin()), &(*ret.end()), dummy);
	size_t rem = (ubuf.data() + ubuf.size() - f_next);
	ubuf.erase(ubuf.begin(), ubuf.end() - rem);
	return ret;
}

template<typename ct>
std::string Session_base<ct>::process_encoded_data() {
	const char* f_next;
	std::string ret = this->conv->in(this->xbuf.data(), this->xbuf.data() + this->xbuf.size(), f_next);
	size_t rem = (this->xbuf.data() + this->xbuf.size() - f_next);
	this->xbuf.erase(this->xbuf.begin(), this->xbuf.end() - rem);
	return ret;
}

// Fill environment
// fillGets(QUERY_STRING) is called from here
// Init of (ue|mp)_regex_cache too (instead of in ctor)
template <class char_type>
void Session_base<char_type>::fill(const char* data, size_t size) {

	this->xbuf.append(data, data + size);
	while (this->xbuf.size()) {
		size_t name_size;
		size_t value_size;
		const char* name;
		const char* value;
		
		if (!protocol::process_param_header(this->xbuf.data(), this->xbuf.size(),
							name, name_size, value, value_size))
			return;

		std::string k(name, name_size);
		std::string v(value, value_size);
		this->xbuf.erase(this->xbuf.begin(), this->xbuf.begin() + name_size + value_size +
							(name_size > 0x7F ? 4 : 1) +
							(value_size > 0x7F ? 4 : 1));
		this->envs.insert(std::make_pair(k, v));
		this->set_charset("UTF-8");
		if (k == "CONTENT_TYPE") {
			if (v.size()) {
				std::string formT("application/x-www-formurl-encoded");
				std::string mpT("multipart/form-data");
				if (!v.compare(0, formT.size(), formT)) {
					if (!this->init_ue())
						throw std::runtime_error("session_base->init_ue");
				} else if (!v.compare(0, mpT.size(), mpT)) {
					boost::xpressive::smatch m;
					if (boost::xpressive::regex_search(v, m, rc().boundary)) {
						if(!this->init_mp("--" + m.str(1)))
							throw std::runtime_error("session_base->init_mp");
					}
				} else {
					throw std::runtime_error("Bad Content-type");
				}
			}
		} else if (k == "QUERY_STRING") {
			fill_ue_oneshot(value, value_size, gets);
		} else if (k == "HTTP_COOKIE") {
			fill_ue_oneshot(value, value_size, cookies);
		}
	}
}

template <class char_type>
void Session_base<char_type>::fill_ue_oneshot(const char* data, size_t size,
	typename Session_base<char_type>::Kv& dest, char val_sep)
{
	// Since this isn't chunked (i.e. it's one-shot), we don't have to worry about partial input.
	// Hence, this function is only half as long as fill_ue(const char*, size_t).
	const char* const data_end = data + size;
	conv.reset(get_conv("url-encoded"));
	while (data != data_end) {
		const char* sep_k = std::find(data, data_end, '=');
		const char* sep_v = std::find(data, data_end, val_sep);
		
		// Check if this is a continuation or not.
		if (sep_k == data_end) 
			return;
		if (sep_v == data_end)
			sep_v = data_end;
		
		this->xbuf = std::string(data, sep_k);
		this->ubuf = this->process_encoded_data();
		T_string name = this->to_unicode();
		this->xbuf = std::string(sep_k + 1, sep_v);
		this->ubuf = this->process_encoded_data();
		T_string value = this->to_unicode();
		if (sep_v != data_end) {
			data = sep_v + 1;
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


