//! \file mosh/fcgi/http/session/session.tcc Template implementations for Session
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


#ifndef FASTCGIPP_HTTP_SESSION_SESSION_TCC
#define FASTCGIPP_HTTP_SESSION_SESSION_TCC

#include <algorithm>
#include <memory>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/xpressive/xpressive.hpp>
#include <mosh/fcgi/boyer_moore.hpp>
#include <mosh/fcgi/http/conv/converter.hpp>
#include <mosh/fcgi/bits/singleton.hpp>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/http/session/session_base.hpp>
#include <mosh/fcgi/http/session/session.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

namespace xpr = boost::xpressive;

// This macro is a workaround for Boost containters which don't have move semantics implemented
// It works by making a temp and then calling .swap()
// Args:
//	t - variable to assign to
//	u - constructor args
#define _mosh_fcgi__move_construct(t, u) do{ decltype(t) _t u; t.swap(_t); }while(0)

template <typename ct, typename pt>
struct Session<ct, pt>::Ue_regex_cache {
	xpr::sregex escaped_hex;
	Ue_regex_cache() {
		escaped_hex = '%' >> (xpr::s1 = xpr::xdigit >> xpr::xdigit);
	}
	virtual ~Ue_regex_cache() { }
};

template <typename ct, typename pt>
struct Session<ct, pt>::Mp_regex_cache {
	// header long line join
	xpr::sregex rfc822_cont;
	// header K: V tokenizer regex
	xpr::sregex rfc822_hdr;
	// for Content-Disposition
	xpr::sregex cd_name; // name="...
	xpr::sregex cd_fname; // filename="...
	xpr::sregex w1; // \b(\w)
	// for Content-Type
	xpr::sregex ct_cs; // charset="...
	Boyer_moore_searcher mp_mixed; // multipart/mixed
	xpr::sregex cte; // valid Content-Transfer-Encoding strings
	xpr::sregex cte_ign; // ignored Content-Transfer-Encodings
	Mp_regex_cache() : mp_mixed("multipart/mixed")
	{
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

template <typename ct, typename pt>
struct Session<ct, pt>::Ue_type {

	/*! @brief Get an instance of the urlencoded's regex cache.
	 * On first invocation, the Ue_regex_cache constructor is called,
	 * which ensures that we have to compile the regexes only once.
	 * With the singleton method, we defer regex compilation to when
	 * multipart parsing actually begins instead of in the Http::Session
	 * constructor.
	 * @returns an instance of the urlencoded's regex cache
	 */
	Ue_regex_cache& rc() {
		return S_rc.instance();
	}
	//! Pointer to last entry
	MP_entry* cur_entry;
	//! At which stage of KV parsing are we?
	enum class State { name, value } state;
	Ue_type() : state(State::name) { }
	virtual ~Ue_type() { }
private:
	//! Regex cache for ue_vars
	static Singleton<Ue_regex_cache> S_rc;
};


template <typename ct, typename pt>
struct Session<ct, pt>::Mp_type {
	/*! @brief Get an instance of the multipart's regex cache.
	 * On first invocation, the Mp_regex_cache constructor is called,
	 * which ensures that we have to comp_ile the regexes only once.
	 * With the singleton method, we defer regex comp_ilation to when
	 * multipart parsing actually begins instead of in the Http::Session
	 * constructor.
	 * @returns an instance of the multipart's regex cache
	 */
	Mp_regex_cache& rc() {
		return S_rc.instance();
	}
	//! Stage of multipart/form-data parsing
	enum class State { header, data } state;
	//! POST boundary for multitype objects
	std::string boundary; // --$boundary (no newline!)
	Boyer_moore_searcher s_bound;
	//! Pointer to last entry
	MP_entry* cur_entry;
	//! @c true if data's Content-type is multipart/mixed (i.e. pass data to fill_mp_mixed)
	bool mixed;

	Mp_type() : state(State::header) { }
	//! Multipart/mixed specifics
	struct Mm_type {
		//! Stage of multipart/mixed parsing
		enum class State { header, data } state;
		MP_mixed_entry* cur_entry;
		//! @c true if --(curentry->boundary)-- was found
		bool stop_parsing;
		Mm_type() : state(State::header) { }
	} mm_vars;
	//! @c true if --boundary-- was found
	bool stop_parsing;
private:
	//! Regex cache for mp_vars
	static Singleton<Mp_regex_cache> S_rc;
};

template <typename ct, typename pt>
bool Session<ct, pt>::init_ue() {
	this->multipart = false;
	this->ue_vars.reset(new Ue_type);
	this->conv.reset(get_conv("url-encoded"));
	return true;
}

template <typename ct, typename pt>
bool Session<ct, pt>::init_mp(const std::string& bound) {
	this->multipart = true;
	this->mp_vars.reset(new Mp_type);
	this->mp_vars->boundary = bound;
	this->mp_vars->s_bound = Boyer_moore_searcher(bound);
	return true;
}

template <typename ct, typename pt>
void Session<ct, pt>::fill_post(const char* data, size_t size) {
	if (this->multipart)
		this->fill_mp(data, size);
	else
		this->fill_ue(data, size);
}

// Precondition: this->ic untouched since this->fill
template <typename ct, typename pt>
void Session<ct, pt>::fill_ue(const char* data, size_t size) {
	const char* const data_end = data + size;
	while (data != data_end) {
		switch (this->ue_vars->state) {
		case Ue_type::State::name:
		{
			const char* _eoh = std::find(data, data_end, '=');
			this->xbuf.append(data, _eoh);
			this->xbuf.append(data, _eoh);
			if (_eoh == data_end)
				return;
			data = _eoh + 1;
			size_t _pct = this->xbuf.rfind('%');
			if (_pct != std::string::npos) {
				xpr::smatch m;
				// check for incomplete buffer
				if (!(
					xpr::regex_search(this->xbuf.substr(_pct, _pct + 3), m, this->ue_vars->rc().escaped_hex)
					|| (_pct + 3 <= this->xbuf.size())
				))
					return;
			}
			this->ubuf += this->process_encoded_data();
			MP_entry cur_fe(this->to_unicode());
			// Prepare pointers for state::data mode
			this->posts[cur_fe.name] << std::move(cur_fe);
			this->ue_vars->cur_entry = &(this->posts[cur_fe.name].last_value());
			this->ue_vars->state = Ue_type::State::value;
			// Clear xbuf and ubuf to avoid unintented conversion results 
			this->xbuf.clear();
			this->ubuf.clear();
		}
		case Ue_type::State::value:
		{
			const char* _eov = std::find(data, data_end, '&');
			this->xbuf.append(data, _eov);
			this->ubuf += this->process_encoded_data();
			this->ue_vars->cur_entry->append_text(this->to_unicode());
			if (_eov != data_end) {
				this->ue_vars->state = Ue_type::State::name;
				data = _eov + 1;
				// Clear xbuf and ubuf to avoid unintented conversion results 
				this->xbuf.clear();
				this->ubuf.clear();
			} else
				return;
		}	
		} /* switch (ue_vars->state) */
	} /* while (data != data_end) */
}

template <typename ct, typename pt>
std::map<std::string, std::string> Session<ct, pt>::mp_read_header(const std::string& buf) const {

	// Inspire by Perl's CGI.pm
	using namespace xpr;

	std::map<std::string, std::string> m;
	std::string s = regex_replace(buf, this->mp_vars->rc().rfc822_cont, " ");
	sregex_iterator m1(s.begin(), s.end(), this->mp_vars->rc().rfc822_hdr);
	sregex_iterator m2;
	for (; m1 != m2; ++m1) {
		std::string canon__1(regex_replace(m1->str(1), this->mp_vars->rc().w1, "\\u($1)", regex_constants::format_perl));
		m[canon__1] = m1->str(2);
	}
	return m;
}

template <typename ct, typename pt>
void Session<ct, pt>::fill_mp(const char* data, size_t size) {
	if (this->mp_vars->stop_parsing)
		return;
	const char* const data_end = data + size;	
	while (data != data_end) {
		switch (this->mp_vars->state) {
		case Mp_type::State::header:
		{
			const char* _clcl = "\r\n\r\n"; // 4*n ~= m*n; don't need boyer-moore
			const char* _eoh = std::search(data, data_end, _clcl, _clcl + 4);
			this->xbuf.append(data, _eoh);
			if (_eoh == data_end)
				return;
			auto header = this->mp_read_header(this->xbuf);
			this->xbuf.clear();
			this->set_charset("UTF-8");
			data = _eoh + 4;
			// Parse the headers
			MP_entry cur_entry;
			{ /* Content-Disposition */
				using namespace xpr;
				smatch m;
				auto& r_cd = header["Content-Disposition"];
				if (regex_match(r_cd, m, this->mp_vars->rc().cd_name)) {
					this->ubuf = m.str(1);
					cur_entry.name = this->to_unicode();
				}
				if (regex_match(r_cd, m, this->mp_vars->rc().cd_fname)) {
					this->ubuf = m.str(1);
					cur_entry.filename = this->to_unicode();
				}
			} /* Content-Disposition */
			{ /* Content-Type */
				using namespace xpr;
				// Check Value
				auto& r_ct = header["Content-Type"];
				const char* _dummy; // dummy
				this->mp_vars->mixed = (this->mp_vars->rc().mp_mixed.search(r_ct.data(), r_ct.size(), _dummy));
				this->mp_vars->mm_vars.stop_parsing = false;
				// Check Attribute charset
				smatch m;
				if (regex_match(r_ct, m, this->mp_vars->rc().ct_cs)) {
					cur_entry.charset = m.str(1);
				}
			} /* Content-Type */
			{ /* Content-Transfer-Encoding */
				using namespace xpr;
				smatch m;
				if (regex_match(header["Content-Transfer-Encoding"], m, this->mp_vars->rc().cte)) {
					std::string s = m.str(1);
					smatch _m0; // dummy
					if (!regex_match(s, _m0, this->mp_vars->rc().cte_ign)) {
						cur_entry.ct_encoding = m.str(1);	
						// Convert to lower case
						std::for_each(cur_entry.ct_encoding.begin(), cur_entry.ct_encoding.end(),
								[&] (char& ch) {
									if ('A' <= ch && ch <= 'Z')
										ch += ('a' - 'A');
								}
						);
					}
				}
			} /* Content-Transfer-Encoding */
			for (auto& h : header) {
				this->ubuf = h.second;
				if (!h.first.compare("Content-Type")) {
					cur_entry.content_type = this->ubuf;

				} else {
					cur_entry.add_header(h.first, this->ubuf);
				}
			}
			if (this->mp_vars->mixed) {
				using namespace xpr;
				// prepare the cur_entry
				MP_mixed_entry cur_mm(std::move(cur_entry));
				smatch m;
				if (regex_match(header["Content-Type"], m, this->rc().boundary)) {
					cur_mm.set_boundary("--" + m.str(1));
				}
				// prepare pointers for fill_mm
				this->mm_posts[cur_mm.name] << std::move(cur_mm);
				this->mp_vars->mm_vars.cur_entry = &(this->mm_posts[cur_mm.name].last_value()); 
				this->mp_vars->mm_vars.stop_parsing = false;
			} else {
				// Prepare pointers for state::data mode
				this->posts[cur_entry.name] << std::move(cur_entry);
				this->mp_vars->cur_entry = &(this->posts[cur_entry.name].last_value());
			}
			// Done with headers. Data time.
			this->xbuf.clear();
			this->mp_vars->state = Mp_type::State::data;
		}
			/* no break statement here */
		case Mp_type::State::data:
		{
			MP_entry& cur = *this->mp_vars->cur_entry;
			if (!this->mp_vars->mixed) {
				Converter* c = get_conv(cur.ct_encoding);
				if (c == 0)
					throw std::runtime_error("Couldn't find a converter for "
								+ cur.ct_encoding);
				this->conv.reset(c);
			}

			const char* bound;
			bool found = this->mp_vars->s_bound.search(data, (data_end - data), bound);
			if (!found) {
				bound = data_end;
			}
			if (this->mp_vars->mixed) {
				this->fill_mm(data, bound - data);
			} else {
				if (!cur.charset.empty()) {
					this->set_charset(cur.charset);
				}
				if (!cur.ct_encoding.empty()) {
					this->xbuf.append(data, bound);
					if (cur.is_file()) {
						cur.append_binary(this->process_encoded_data());
					} else {
						this->ubuf += this->process_encoded_data();
						cur.append_text(this->to_unicode());
					}
				} else {
					if (cur.is_file()) {
						cur.append_binary(data, bound);
					} else {
						this->ubuf.append(data, bound);
						cur.append_text(this->to_unicode());
					}
				}
			}
			
			if (found) {			
				std::string t = this->mp_vars->boundary + "--";
				if (((data_end - bound) >= static_cast<ptrdiff_t>(t.size()))
				&& t.compare(0, t.size(), bound) == 0) {
					this->mp_vars->stop_parsing = true;
					return;
				} 
				data = bound + this->mp_vars->boundary.size();
				this->mp_vars->state = Mp_type::State::header;
			} else {
				return;
			}
		}
		} /* switch (mp_vars->state) */
	} /* while (data != data_end) */
}

template <typename ct, typename pt>
void Session<ct, pt>::fill_mm(const char* data, size_t size) {
	if (this->mp_vars->mm_vars.stop_parsing)
		return;
	const char* const data_end = data + size;	
	while (data != data_end) {
		switch (this->mp_vars->mm_vars.state) {
		case Mp_type::Mm_type::State::header:
		{
			const char* _clcl = "\r\n\r\n"; // 4*n ~= m*n; don't need boyer-moore
			const char* _eoh = std::search(data, data_end, _clcl, _clcl + 4);
			this->xbuf.append(data, _eoh);
			// No terminator found; keep on buffering
			if (_eoh == data_end)
				return;
			auto header = this->mp_read_header(this->xbuf);
			this->xbuf.clear();
			data = _eoh + 4;
			// Parse the headers
			MP_entry cur_mp;
			{ /* Content-Disposition */
				using namespace xpr;
				smatch m;
				if (regex_match(header["Content-Disposition"], m, this->mp_vars->rc().cd_fname)) {
					this->ubuf = m.str(1);
					cur_mp.filename = this->to_unicode();
				}
			} /* Content-Disposition */
			{ /* Content-Type */
				using namespace xpr;
				// Check Attribute charset
				smatch m;
				if (regex_match(header["Content-Type"], m, this->mp_vars->rc().ct_cs)) {
					cur_mp.charset = m.str(1);
				}
			} /* Content-Type */
			{ /* Content-Transfer-Encoding */
				using namespace xpr;

				smatch m;
				if (regex_match(header["Content-Transfer-Encoding"], m, this->mp_vars->rc().cte)) {
					std::string s = m.str(1);
					smatch _m0; // dummy
					if (!regex_match(s, _m0, this->mp_vars->rc().cte_ign)) {
						cur_mp.ct_encoding = std::move(s);
					}
				}
			}
			for (auto& h : header) {
				this->ubuf = h.second;
				if (!h.first.compare("Content-Type")) {
					cur_mp.content_type = this->ubuf;
				} else {
					cur_mp.add_header(h.first, this->ubuf);
				}
			}
			this->mp_vars->mm_vars.cur_entry->add_value(std::move(cur_mp));
			// Done with headers. Data time.
			this->mp_vars->mm_vars.state = Mp_type::Mm_type::State::data;
		}
			/* no break statement here */
		case Mp_type::Mm_type::State::data:
		{
			MP_mixed_entry& cur = *this->mp_vars->mm_vars.cur_entry;
			MP_entry& mp = cur.last_value();
			if (!mp.ct_encoding.empty()) {
				Converter* c = get_conv(mp.ct_encoding);
				if (c == 0)
					throw std::runtime_error("Couldn't find a converter for "
								+ mp.ct_encoding);
				this->conv.reset(c);
			}
			
			const char* bound;
			bool found = cur.boundary_searcher().search(data, (data_end - data), bound);
			if (!found)
				bound = data_end;
			if (!mp.charset.empty()) {
				this->set_charset(mp.charset);
			}	
			if (!mp.ct_encoding.empty()) {
				this->xbuf.append(data, bound);
				if (mp.is_file()) {
					mp.append_binary(this->process_encoded_data());
				} else {
					this->ubuf += this->process_encoded_data();
					mp.append_text(this->to_unicode());
				}
			} else {
				if (mp.is_file()) {
					mp.append_binary(data, bound);
				} else {
					this->ubuf.append(data, bound);
					mp.append_text(this->to_unicode());
				}
			}
			
			if (found) {			
				std::string t = cur.boundary() + "--";
				if (((data_end - bound) >= static_cast<ptrdiff_t>(t.size()))
				&& t.compare(0, t.size(), bound) == 0) {
					this->mp_vars->mm_vars.stop_parsing = true;
					return;
				} else {
					data = bound + cur.boundary().size();
					this->mp_vars->mm_vars.state = Mp_type::Mm_type::State::header;
				}
			} else {
				return;
			}
		}
		} /* switch (mp_vars->mm_vars.state) */
	} /* while (data != data_end) */
}

#undef _mosh_fcgi__move_construct

} // namespace http

MOSH_FCGI_END

#endif


