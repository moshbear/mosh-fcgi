//! @file http/session.cpp Defines functions in http/session/funcs.hpp
/***************************************************************************
* Copyright (C) 2012-3 m0shbear                                            *
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

#include <algorithm>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <cstring>
#include <mosh/fcgi/protocol/funcs.hpp>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/boyer_moore.hpp>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/http/cookie.hpp>
#include <mosh/fcgi/http/conv/url.hpp>
#include <mosh/fcgi/http/session/funcs.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/singleton.hpp>
#include <src/namespace.hpp>
#include <src/http/mime.hpp>
#include <src/utility.hpp>

namespace {

#include "lut.hpp"

//
// Miscellany.
//

std::string char_to_hex_string(char ch) {
	static const char hex[] = "0123456789ABCDEF";
	int c(widen_cast<int>(ch));
	std::string buf("0x..");
	buf[2] = hex[(c & 0xF0) >> 4];
	buf[3] = hex[(c & 0x0F)];
	return buf;
}

}

struct Mp_regex_cache {
	MOSH_FCGI::Boyer_moore_searcher mp_mixed;
	
	Mp_regex_cache() : mp_mixed("multipart/mixed") {
	}
	virtual ~Mp_regex_cache() { }
};

// wrap the singletons in a struct to avoid default initialiazation
struct _si {
	static SRC::Singleton<Mp_regex_cache> _m_rc;
};

Mp_regex_cache& m_rc() { return _si::_m_rc.instance(); }


std::map<std::string, std::string> read_mime_header(std::string const& buf) {
	
	std::string nocont;

	{
		bool usable;
		std::tie(usable, nocont) = fold_long_headers_and_strip_comments(buf);
		if (!usable)
			throw std::invalid_argument(nocont);
	}

	// split <header>: <value>\r\n
	
	std::map<std::string, std::string> ret;
	{
		std::string::size_type nc_pos = 0;
		std::string::size_type crlf_pos = 0;
		std::string::size_type nc_size = nocont.size();

		std::string kbuf;
		std::string vbuf;

		bool prev_w = false;
		bool cur_w;
		unsigned t_state;
		enum { TOKEN, INTER, VALUE } state;

		
		while (nocont[nc_pos] == ' ')
			++nc_pos;

		for (const char* ncp = nocont.data() ; nc_pos < nc_size ; ++nc_pos) {
			switch (state) {
			case TOKEN:
				t_state = tokens[widen_cast<int>(ncp[nc_pos])];
				cur_w = !!(t_state & tok_flags::wordchar) ;
				if (cur_w && !prev_w) 
					kbuf += narrow_cast<char>(upper[widen_cast<int>(ncp[nc_pos])]);
				if (t_state & tok_flags::none) {
					state = INTER;
					while (ncp[nc_pos + 1] == ' ')
						++nc_pos;
				}
				prev_w = cur_w;
				break;
			case INTER: 
				if (ncp[nc_pos] != ':')
					throw std::invalid_argument("Malformed header: expected ':', got '" + nocont[nc_pos] + "'");
			
				while (ncp[nc_pos + 1] == ' ')
					++nc_pos;
				state = VALUE;
				break;
			case VALUE: // Unstructured field-body (RFC 822 (S) 3.1.3)
				crlf_pos = nocont.find("\r\n", nc_pos);
				
				if (crlf_pos != std::string::npos) 
					vbuf = std::move(ncont.substr(nc_pos, crlf_pos - nc_pos));	
				else
					vbuf = std::move(nocont.substr(nc_pos, crlf_pos));
				ret[kbuf] = vbuf;
				kbuf.clear();
				prev_w = false;
				if (crlf_pos == std::string::npos) {
					nc_pos = nc_size;
					break;
				}
				// skip through leading spaces for next iteration
				nc_pos = crlf_pos + 1;
				while (ncp[nc_pos + 1] == ' ')
					++nc_pos;
				prev_w = false;
				state = TOKEN;
				break;
			}
		}
	}

	return ret;
}

}

MOSH_FCGI_BEGIN

namespace http {

namespace session {

ssize_t process_urlencoded_kv(const char* data, size_t size, u_string& k, u_string& v) {
	const char* const data_end = data + size;
	const char* sep_k = std::find(data, data_end, '=');
	const char* sep_v = std::find(data, data_end, '&');
	
	static Url c;

	if (sep_k == data_end)
		return -1;
	if (sep_v == data_end)
		sep_v = data_end;
	
	const char* p;
	k = c.in(data, sep_k, p);
	if (p != sep_k) 
		return -1;
	v = c.in(sep_k + 1, sep_v, p);
	if (p != sep_v)
		return -1;
	return std::distance(data_end, sep_v);
}

ssize_t process_cookies(const char* data, size_t size, std::map<std::string, form::Entry<char, Cookie>>& kv, Cookie& g) {
	Cookie* last = &g;
	const char* const data_end = data + size;
	const char* sep_v = nullptr;
	while (data != data_end) {
		const char* sep_k = std::find(data, data_end, '=');
		if (sep_k == data_end)
			return -1;
		// trim whitespace before checking for quote
		const char* start_v = sep_k;
		while (start_v != data_end && !(tokens[widen_cast<int>(*start_v)] & tok_flags::lwspchar)
			++start_v;
		if (start_v == data_end)
			throw std::invalid_argument("Unexpected end of cookie data");

		bool seen_quot = false;
		if (*start_v == '"') {
			seen_quot = true;
			++start_v;
		}

		sep_v = std::find(start_v, data_end, seen_quot ? '"' : ';');
		if (sep_v == data_end) {
			if (seen_quot) 
				throw std::invalid_argument("Malformed cookie header");
			else 
				sep_v = data_end;
		}

		std::string k(data, sep_k);
		std::string v(start_v, sep_v);

		seen_quot = false;

		if (k[0] == '$') {
			if (k == "$Path")
				last->path = v;
			else if (k == "$Domain")
				last->domain = v;
			else if (k == "$Version") {
				if (v != "1") 
					throw std::invalid_argument("Unsupported cookie version");
			}
		} else {
			if (kv.find(k) == kv.end()) {
				form::Entry<char, Cookie> c(k, Cookie(k, v));
				c.enable_unique_mode();
				kv[k] = std::move(c);
			} else {
				kv[k] << std::move(Cookie(k, v));
			}
			last = &(kv[k].last_value());
		}
	}
	return std::distance(data_end, sep_v);
} 

void do_param(std::pair<std::string, std::string> const& p,
		std::function<bool ()> ue_init,
		std::function<bool (std::string)> mp_init,
		std::function<void (const char*, size_t)> do_gets,
		std::function<void (const char*, size_t)> do_cookies)
{
	if (!ue_init || !mp_init || !do_gets || !do_cookies)
		throw std::invalid_argument("Undefined functors");
	
	std::string const& k = p.first;
	std::string const& v = p.second;
	//this->charset() = "";
	if (k == "CONTENT_TYPE") {
		if (v.size()) {
			bool flag = false;
			auto try_init =	[&v, &flag](std::string const& str, std::function<bool ()> func,
						std::string const& ex)
			{
				if (v.compare(0, str.size(), str))
					return;
				if (!func())
					throw std::runtime_error(ex);
				flag = true;
			};
			
			try_init("application/x-www-formurl-encoded", ue_init, "session_base->init_ue");
			try_init("multipart/form-data", std::bind(mp_init, boundary_from_ct(v)), "session_base->init_mp");
			if (!flag)
				throw std::invalid_argument("Unrecognized Content-type \"" + v + "\"");
		}
	} else if (k == "QUERY_STRING") {	
		do_gets(sign_cast<const char*>(v.data()), v.size());
	} else if (k == "HTTP_COOKIE") {
		do_cookies(sign_cast<const char*>(v.data()), v.size());
	}
}

void do_mp_headers(std::string const& buf,
		std::function<void (u_string const&)> name_func,
		std::function<void (u_string const&)> fname_func,
		std::function<void (std::string const&)> cs_func,
		std::function<void (bool)> mixed_func,
		std::function<void (std::string const&)> cte_func,
		std::function<void (std::string const&, std::string const&)> h_func)
{
	if ((!name_func) || (!fname_func)
	|| (!cs_func) || (!mixed_func) || (!cte_func))
		throw std::invalid_argument("Undefined functors");
	
	std::map<std::string, std::string> header = read_mime_header(buf);
	{ /* Content-Disposition */
		auto _r_c = header.find("Content-Disposition");
		if (_r_c == header.end() || _r_c->second.empty())
			throw_parser_error("Missing Content-Disposition header");
		auto& r_cd = _r_c->second;
		auto cd_params = mime::get_mime_params(r_cd);
		decltype(cd_params.find("")) param;
		if ((param = cd_params.find("name")) != cd_params.end()) {
			std::string s = param->second;
			const uchar* _s_ = sign_cast<const uchar*>(s.data());
			name_func(u_string(_s_, _s_ + s.size()));
		}
		if ((param = cd_params.find("filename")) != cd_params.end()) {
			std::string s = param->second;
			const uchar* _s_ = sign_cast<const uchar*>(s.data());
			fname_func(u_string(_s_, _s_ + s.size()));
		}
	}
	{	/* Content-Type */
		auto _r_c = header.find("Content-Type");
		if (_r_c == header.end() || _r_c->second.empty())
			throw_parser_error("Missing Content-Type header");
		auto& r_ct = _r_c->second;
		auto ct_params = mime::get_mime_params(r_ct);
		decltype(ct_params.find("")) param;
		const char* _dummy; // dummy
		mixed_func(m_rc().mp_mixed.search(r_ct.data(), r_ct.size(), _dummy));
		// Check Attribute charset
		if ((param = ct_params.find("charset")) != ct_params.end()) 
			cs_func(params->second);
	}
	{ /* Content-Transfer-Encoding */
		auto _r_c = header.find("Content-Transfer-Encoding");
		if (_r_c != header.end() && !_r_c->second.empty()) {
			auto& r_cte = _r_c->second;
			std::string cte = fetch_cte(r_cte);
			if (!cte.empty()) {
				if (!match_cte(r_cte, cte_flags::ign))
					cte_func(cte);
			}
		}
	}
	for (auto& h : header)
		h_func(h.first, h.second);
}

std::string boundary_from_ct(std::string const& ct) {
	
	// Parses the grammar boundary=("(?:boundary_special|alpha|digit)*" | (?:alpha|digit|${boundary_special - tspecial}))

	return SRC::mime::filtering::filter(SRC::mime::get_mime_param(ct, "boundary"),
						SRC::mime::filtering::field::content_type, SRC::mime::filtering::subfield_content_type::boundary);
}

}

}

MOSH_FCGI_END
