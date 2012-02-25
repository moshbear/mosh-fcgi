//! @file http/session.cpp Defines functions in http/session/funcs.hpp
/***************************************************************************
* Copyright (C) 2012 m0shbear                                              *
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
#include <stdexcept>
#include <string>
#include <map>
#include <utility>
#include <cstring>
#include <boost/xpressive/xpressive.hpp>
#include <mosh/fcgi/protocol/funcs.hpp>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/boyer_moore.hpp>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/http/cookie.hpp>
#include <mosh/fcgi/http/conv/url.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/singleton.hpp>
#include <src/namespace.hpp>
namespace {

// Initialize the regex cache
namespace xpr = boost::xpressive;

struct Regex_cache {
	xpr::sregex boundary;
	Regex_cache() :
		boundary("boundary=" >> !xpr::as_xpr('"') >> (xpr::s1 = +(~(xpr::set = '"', ';', ','))) >> !xpr::as_xpr('"'))
	{ }
	virtual ~Regex_cache() { }
};

struct Ue_regex_cache {
	xpr::cregex escaped_hex;
	Ue_regex_cache() {
		escaped_hex = '%' >> (xpr::xdigit >> xpr::xdigit);
	}
	virtual ~Ue_regex_cache() { }
};
	
struct Mp_regex_cache {
	xpr::sregex rfc822_cont;
	xpr::sregex rfc822_hdr;
	xpr::sregex cd_name;
	xpr::sregex cd_fname;
	xpr::sregex w1;
	xpr::sregex ct_cs;
	MOSH_FCGI::Boyer_moore_searcher mp_mixed;
	xpr::sregex cte;
	xpr::sregex cte_ign;
	
	Mp_regex_cache() : mp_mixed("multipart/mixed") {
#define MIME_NOQUOTE _w|'!'|'#'|'\''|'*'|'+'|','|'.'|'^'|'`'|'{'|'}'|'|'|'~'
		using namespace xpr;
	
		rfc822_hdr = (s1 = +(set[MIME_NOQUOTE|'-'])) >> ':' >> *_s >> (s2 = *(~_ln));
		rfc822_cont = _ln >> +_s;
		
		cd_name = (_s | ';') >> icase("name=\"") >> (s1 = *(~(set = '"'))) >> '"';
		cd_fname = (_s | ';') >> icase("filename=") >> ( s1 = ( '"' >> *(~(set = '"')) >> '"')
							     | (*set[MIME_NOQUOTE]));
		w1 = _b >> (s1 = _w);

		ct_cs = (_s | ';') >> icase("charset=") >> !as_xpr('"') >> (s1 = set[_w|'('|')'|'+'|'-'|'.'|':'|'_']) >> !as_xpr('"');
		
		cte = (s1 = icase(as_xpr("quoted-printable")|"base64"|"8bit"|"binary"));
		cte_ign = icase(as_xpr("8bit")|"binary");

#undef MIME_NOQUOTE
	}
	virtual ~Mp_regex_cache() { }
};

struct _si {
static SRC::Singleton<Regex_cache> _rc;
static SRC::Singleton<Ue_regex_cache> _u_rc;
static SRC::Singleton<Mp_regex_cache> _m_rc;
};

Regex_cache& rc() { return _si::_rc.instance(); }
Ue_regex_cache& u_rc() { return _si::_u_rc.instance(); }
Mp_regex_cache& m_rc() { return _si::_m_rc.instance(); }

std::map<std::string, std::string> read_mime_header(std::string const& buf) {
	std::map<std::string, std::string> m;
	std::string s = xpr::regex_replace(buf, m_rc().rfc822_cont, " ");
	xpr::sregex_iterator m1(s.begin(), s.end(), m_rc().rfc822_hdr);
	xpr::sregex_iterator m2;
	for (; m1 != m2; ++m1) {
		m[xpr::regex_replace(m1->str(1), m_rc().w1, "\\u($1)", xpr::regex_constants::format_perl)] = m1->str(2);
	}
	return m;
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
	const char ws[] = " \t";
	const char* sep_v = nullptr;
	while (data != data_end) {
		const char* sep_k = std::find(data, data_end, '=');
		if (sep_k == data_end)
			return -1;
		// trim whitespace before checking for quote
		const char* start_v = sep_k;
		while (start_v != data_end) {
			if (strchr(ws, *start_v) != NULL)
				++start_v;
		}
		if (start_v == data_end)
			throw std::runtime_error("Unexpected end of cookie data");

		bool seen_quot = false;
		if (*start_v == '"') {
			seen_quot = true;
			++start_v;
		}

		sep_v = std::find(start_v, data_end, seen_quot ? '"' : ';');
		if (sep_v == data_end) {
			if (seen_quot) 
				throw std::runtime_error("Malformed cookie header");
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

void do_fill(u_string& buf, std::map<std::string, std::string>& env,
		std::function<bool ()> ue_init,
		std::function<bool (std::string)> mp_init,
		std::function<void (const char*, size_t)> do_gets,
		std::function<void (const char*, size_t)> do_cookies)
{
	if ((!ue_init) || (!mp_init) || (!do_gets) || (!do_cookies))
		throw std::invalid_argument("Undefined functors");
	
	while (buf.size()) {
		std::pair<std::string, std::string> params;
		ssize_t to_erase = protocol::process_param_record(buf.data(), buf.size(), params);
		if (to_erase == -1)
			return;
		buf.erase(0, to_erase);
		env.insert(params);
		//this->charset() = "";
		std::string& k = params.first;
		std::string& v = params.second;
		if (k == "CONTENT_TYPE") {
			if (v.size()) {
				std::string formT("application/x-www-formurl-encoded");
				std::string mpT("multipart/form-data");
				if (!v.compare(0, formT.size(), formT)) {
					if (!ue_init())
						throw std::runtime_error("session_base->init_ue");
				} else if (!v.compare(0, mpT.size(), mpT)) {
					xpr::smatch m;
					if (xpr::regex_search(v, m, rc().boundary)) {
						if (!mp_init("--" + m.str(1)))
							throw std::runtime_error("session_base->init_mp");
					}
				} else {
					throw std::runtime_error("Bad Content-type");
				}
			}
		} else if (k == "QUERY_STRING") {	
			do_gets(sign_cast<const char*>(v.data()), v.size());
		} else if (k == "HTTP_COOKIE") {
			do_cookies(sign_cast<const char*>(v.data()), v.size());
		}
	}
}

void do_headers(std::string const& buf,
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
		xpr::smatch m;
		auto _r_c = header.find("Content-Disposition");
		if (_r_c == header.end() || _r_c->second.empty())
			throw std::runtime_error("Missing Content-Disposition header");
		auto& r_cd = _r_c->second;
		if (xpr::regex_match(r_cd, m, m_rc().cd_name)) {
			std::string s = m.str(1);
			const uchar* _s_ = sign_cast<const uchar*>(s.data());
			name_func(u_string(_s_, _s_ + s.size()));
		}
		if (xpr::regex_match(r_cd, m, m_rc().cd_fname)) {
			std::string s = m.str(1);
			const uchar* _s_ = sign_cast<const uchar*>(s.data());
			fname_func(u_string(_s_, _s_ + s.size()));
		}
	}
	{	/* Content-Type */
		// Check Value
		auto _r_c = header.find("Content-Type");
		if (_r_c == header.end() || _r_c->second.empty())
			throw std::runtime_error("Missing Content-Type header");
		auto& r_ct = _r_c->second;
		const char* _dummy; // dummy
		mixed_func(m_rc().mp_mixed.search(r_ct.data(), r_ct.size(), _dummy));
		// Check Attribute charset
		xpr::smatch m;
		if (xpr::regex_match(r_ct, m, m_rc().ct_cs)) 
			cs_func(m.str(1));
	}
	{ /* Content-Transfer-Encoding */
		xpr::smatch m;
		auto _r_c = header.find("Content-Transfer-Encoding");
		if (_r_c != header.end() && !_r_c->second.empty()) {
			auto& r_cd = _r_c->second;
			if (xpr::regex_match(r_cd, m, m_rc().cte)) {
				std::string s = m.str(1);
				xpr::smatch _m0; // dummy
				if (!xpr::regex_match(s, _m0, m_rc().cte_ign)) {
					std::string cte = m.str(1);	
					// Convert to lower case
					std::for_each(cte.begin(), cte.end(),
							[] (char& ch) {
								if ('A' <= ch && ch <= 'Z')
									ch += ('a' - 'A');
							}
					);
					cte_func(cte);
				}
			}
		}
	}
	for (auto& h : header)
		h_func(h.first, h.second);
}


}
}

MOSH_FCGI_END
