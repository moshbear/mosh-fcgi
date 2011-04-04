//! \file http/http.tcc Template implementations for http
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*               2007 Eddie                                                 *
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


#ifndef FASTCGIPP_HTTP_TCC
#define FASTCGIPP_HTTP_TCC

#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <boost/regex.hpp>
#include <fastcgipp-mosh/protocol.hpp>
#include <fastcgipp-mosh/unicode.hpp>
#include <fastcgipp-mosh/http/form.hpp>
#include <fastcgipp-mosh/bits/append_or_assign.hpp>
#include <fastcgipp-mosh/bits/boyer_moore.hpp>
#include <fastcgipp-mosh/bits/singleton.hpp>
#include <fastcgipp-mosh/bits/http/url.hpp>

#include <fastcgipp-mosh/http/http.hpp>

namespace Fastcgipp_m0sh
{
namespace Http
{

using namespace std;
using namespace boost;

// Note: these caches should be called only from a singleton
// Keeping them as non-static non-singleton class objects defeats their purpose.
// As does using them in a static context, because then the overhead is added to
// the constructor instead of on-first-use.

// for Environment<char_type, post_vec_type, data_vec_type> regexes
struct regex_cache {
	regex boundary;
	regex_cache() :
		boundary("boundary=\"?([^\";,]+)\"?")
	{ }
};
// for Environment<char_type, post_vec_type, data_vec_type>::ue_vars regexes
struct ue_regex_cache {
	regex escaped_hex;
	ue_regex_cache() :
		escaped_hex("%([[:xdigit:][:xdigit:]])")
	{ }
};
// for Environment<char_type, post_vec_type, data_vec_type>::mp_vars regexes
struct mp_regex_cache {
	// header long line join
	regex rfc822_cont;
	// header K: V tokenizer regex
	regex rfc822_token;
	// for Content-Disposition
	regex cd_name; // name="...
	regex cd_fname; // filename="...
	regex w1; // \b(\w)
	// for Content-Type
	Boyer_moore_searcher mp_mixed; // multipart/mixed

	mp_regex_cache() :
		rfc822_cont("\r\n\\s+", regex::perl | regex::no_mod_m),
		rfc822_token("([-\\w!#$%&\\'*+.^_\\`|{}~]+):\\s+([^\r\n]*)", regex::perl | regex::mod_x),
		cd_name("[\\s;]name=\"([^\"]*)\""),
		cd_fname(" filename=((\"[^\"]*\")|([a-z\\d!#'*+,.^_`{}|~]*))", regex::perl | regex::icase),
		w1("\\b(\\w)"),
		mp_mixed("multipart/mixed")
	{ }
};

// Fill environment
// fillGets(QUERY_STRING) is called from here
// Init of (ue|mp)_regex_cache too (instead of in ctor)
template <class char_type, class post_vec_type,class data_vec_type>
bool Environment<char_type, post_vec_type, data_vec_type>::fill(const char* data, size_t size) {
	while (size) {
		size_t nameSize;
		size_t valueSize;
		const char* name;
		const char* value;

		if (!Protocol::processParamHeader(data, size, name, nameSize, value, valueSize))
			return false;
		size -= value - data + valueSize;
		data = value + valueSize;
		string k(name, nameSize);
		string v(value, valueSize);
		environment.insert(make_pair(k, v));
		if (k == "CONTENT_TYPE") {
			if (v.size()) {
				string formT("application/x-www-formurl-encoded");
				string mpT("multipart/form-data");
				if (!v.compare(0, formT.size(), formT)) {
					multipart = false;
				} else if (!v.compare(0, mpT.size(), mpT)) {
					multipart = true;
					smatch m;
					if (regex_match(v, m, rc().boundary)) {
						string s(m[1].first, m[1].second);
						mp_vars.boundary = "--" + s;
						mp_vars._bound = Boyer_moore_searcher(mp_vars.boundary);
					}
				} else {
					throw runtime_error("Bad Content-type");
				}
			}
		} else if (k == "QUERY_STRING") {
			fillGets(value, valueSize);
		}				
	}
	return true;
}


template <class char_type, class post_vec_type,class data_vec_type>
void Environment<char_type, post_vec_type, data_vec_type>::fillGets(const char* data, size_t size) {
	using namespace Url;
	// Since this isn't chunked (i.e. it's one-shot), we don't have to worry about partial input.
	// Hence, this function is only half as long as fillUrlEncoded(const char*, size_t).
	const char* const dataEnd = data + size;
	while (data != dataEnd) {
		const char* sepK = memchr(data, '=', size);
		const char* sepV = memchr(data, '&', size);
		
		// Check if this is a continuation or not.
		if (sepK == 0) 
			return;
		if (sepV == 0)
			sepV = dataEnd;
		TString name = decode<char_type>::_(data, sepK);
		TString value = decode<char_type>::_(sepK + 1, sepV);
		if (sepV != dataEnd) {
			data = sepV + 1;
			size = dataEnd - data;
		} else
			data = dataEnd;
		if (gets.find(name) == gets.end()) {
			Form_entry e(name, value);
			e.enable_uniqueness_mode();
			this->gets[name] = e;
		} else {
			this->gets[name] << value;
		}
	}
}

template <class char_type, class post_vec_type,class data_vec_type>
void Environment<char_type, post_vec_type, data_vec_type>::fillPost(const char* data, size_t size) {
	if (multipart)
		fillMultipart(data, size);
	else
		fillUrlEncoded(data, size);
}
	
template <class char_type, class post_vec_type,class data_vec_type>
void Environment<char_type, post_vec_type, data_vec_type>::fillUrlEncoded(const char* data, size_t size) {
	const char* const dataEnd = data + size;
	while (data != dataEnd) {
		switch (ue_vars.state) {
		case ue_type::ues_name:
		{
			const char* _eoh = memchr(data, '=', size);
			if (_eoh == 0)
				_eoh = data + size;
			append_or_assign(&buf, string(data, _eoh), !buf.empty());
			if (_eoh == data + size)
				return;
			data = _eoh + 1;
			size = dataEnd - data;	
			size_t _pct = buf.rfind('%');
			if (_pct != string::npos) {
				smatch m;
				// check for incomplete buffer
				if (!(
					regex_match(buf.substr(_pct, _pct + 3), m, ue_vars.rc().escaped_hex)
					|| (_pct + 3 <= buf.size())
				))
					return;
			}
			Form_entry cur_fe(Url::decode<char_type>::_(buf));
			buf.clear();
			// Prepare pointers for mps_data mode
			posts[cur_fe.name] << cur_fe;
			ue_vars.cur_entry = &(posts[cur_fe.name].last_value());
		}
		case ue_type::ues_value:
		{
			const char* _eov = memchr(data, '&', size);
			if (_eov == 0)
				_eov == data + size;
			unsigned miss = 0;
			append_or_assign(&buf, string(data, _eov), !buf.empty());
			ue_vars.cur_entry->append(Url::decode<char_type>::_(buf, miss));
			buf.erase(buf.begin(), buf.end() - (miss > 0 ? 3 - miss : 0));
			if (_eov != (data + size)) {
				ue_vars.state = ue_type::ues_name;
				data = _eov + 1;
				size = dataEnd - data;
			} else
				return;
		}	
		} /* switch (ue_vars.state) */
	} /* while (data != dataEnd) */

}

template <class char_type, class post_vec_type,class data_vec_type>
map<string, string> Environment<char_type, post_vec_type, data_vec_type>::mp_read_header(const string& buf) const {
	map<string, string> m;
	// C++ version of multipartHeader in Perl5 CGI.pm

	string s = regex_replace(buf, mp_vars.rc().rfc822_cont, " ", format_perl);
	sregex_iterator m1(s.begin(), s.end(), mp_vars.rc().rfc822_token);
	sregex_iterator m2;
	for (; m1 != m2; ++m1) {
		string canon__1(regex_replace((*m1)[1].str(), mp_vars.w1, "\\u($1)"));
		m[canon__1] = (*m1)[2].str();
	}
	return m;
}


template <class char_type, class post_vec_type,class data_vec_type>
void Environment<char_type, post_vec_type, data_vec_type>::fillMultipart(const char* data, size_t size) {
	using namespace Unicode;
	if (mp_vars.stop_parsing)
		return;
	const char* const dataEnd = data + size;	
	while (data != dataEnd) {
		switch (mp_vars.state) {
		case mp_type::mps_header:
		{
			const char* _clcl = "\r\n\r\n"; // 4*n ~= m*n; don't need boyer-moore
			const char* _eoh = search(data, data + size, _clcl, _clcl + 4);
			append_or_assign(&buf, string(data, _eoh), !buf.empty());
			if (_eoh == data + size)
				return;
			map<string, string> header = mp_read_header(buf);
			buf.clear();
			data = _eoh + 4;
			size = dataEnd - data;
				// Parse the headers
			smatch m;
			MP_entry cur_entry;
			{ /* Content-Disposition */
				const string& r_cd = header["Content-Disposition"];
				if (regex_match(r_cd, m, mp_vars.rc().cd_name)) {
					cur_entry.name = utfIn<char_type>(string(m[1].first, m[1].second));
				}
				if (regex_match(r_cd, m, mp_vars.rc().cd_fname)) {
					cur_entry.filename = utfIn<char_type>(string(m[1].first, m[1].second));
				}
			} /* Content-Disposition */
			{ /* Content-Type */
				const string& r_ct = header["Content-Type"];
				const char* mp_varsm_res; // dummy
				mp_vars.mixed = (mp_vars.rc().mp_mixed.search(r_ct.data(), r_ct.size(), mp_varsm_res));
			} /* Content-Type */
			for (map<string, string>::const_iterator it = header.begin(); it != header.end(); ++it) {
				if (!it->first.compare("Content-Type")) {
					cur_entry.content_type = utfIn<char_type>(it->second);
				} else {
					cur_entry.add_header(it->first, utfIn<char_type>(it->second));
				}
			}
			if (mp_vars.mixed) {
				// prepare the cur_entry
				MP_mixed_entry cur_mm(cur_entry);
				smatch m;
				if (regex_match(mp_vars.header["Content-Type"], m, rc().boundary)) {
					string s(m[1].first, m[1].second);
					cur_mm.set_boundary("--" + s);
				}
				// prepare pointers for fillMultipartMixed
				multipart_mixed_posts[cur_mm.name] << cur_mm;
				mp_vars.mm_vars.cur_entry = &(multipart_mixed_posts[cur_mm.name].last_value());
				mp_vars.mm_varstop_parsing = false;
			} else {
				// Prepare pointers for mps_data mode
				multipart_posts[cur_entry.name] << cur_entry;
				mp_vars.cur_entry = &(multipart_posts[cur_entry.name].last_value());
			}
			// Done with headers. Data time.
			mp_vars.state = mp_type::mps_data;
		}
			/* no break statement here */
		case mp_type::mps_data:
		{
			const char* bound;
			if (mp_vars._bound.search(data, size, bound)) {
				string t = mp_vars.boundary + "--";
				if (t.compare(0, t.size(), bound) == 0) {
					mp_vars.cur_entry->append_data(data, bound);
					mp_vars.stop_parsing = true;
					return;
				}
				if (mp_vars.mixed) {
					fillMultipartMixed(data, bound);
				} else {
					mp_vars.cur_entry->append_data(data, bound);
				}
				data = bound + mp_vars.bound.size();
				data = dataEnd - data;
				mp_vars.state = mp_type::mps_header;
				break;
			}
			if (mp_vars.mixed) {
				fillMultipartMixed(data, dataEnd);
			} else {
				mp_vars.cur_entry->append_data(data, dataEnd);
			}
			dataEnd = data;
		}
		} /* switch (mp_vars.state) */
	} /* while (data != dataEnd) */
}

template <class char_type, class post_vec_type,class data_vec_type>
void Environment<char_type, post_vec_type, data_vec_type>::fillMultipartMixed(const char* data, size_t size) {
	using namespace Unicode;
	if (mp_vars.mm_vars.stop_parsing)
		return;
	const char* const dataEnd = data + size;	
	while (data != dataEnd) {
		switch (mp_vars.mm_vars.state) {
		case mp_type::mm_type::mpmixs_header:
		{
			mp_vars.mm_vars.header.clear();
			const char* _clcl = "\r\n\r\n"; // 4*n ~= m*n; don't need boyer-moore
			const char* _eoh = search(data, data + size, _clcl, _clcl + 4);
			// No terminator found; keep on buffering
			if (_eoh == data + size) {
				append_or_assign(&buf, string(data, data + size), !buf.empty());
				return;
			} else
				append_or_assign(&buf, string(data, _eoh), !buf.empty());
			map<string, string> header = mp_read_header(buf);
			buf.clear();
			data = _eoh + 4;
			size = dataEnd - data;
				// Parse the headers
			smatch m;
			MP_entry cur_mp;
			{ /* Content-Disposition */
				const string& r_cd = header["Content-Disposition"];
				if (regex_match(r_cd, m, mp_vars.rc().cd_fname)) {
					cur_mp.filename = utfIn<char_type>(string(m[1].first, m[1].second));
				}
			} /* Content-Disposition */
			for (map<string, string>::const_iterator it = header.begin(); it != header.end(); ++it) {
				if (!it->first.compare("Content-Type")) {
					cur_mp.content_type = utfIn<char_type>(it->second);
				} else {
					cur_mp.add_header(it->first, utfIn<char_type>(it->second));
				}
			}
			(*mp_vars.mm_vars.cur_entry) << cur_mp;
			// Done with headers. Data time.
			mp_vars.mm_vars.mp_vars.state = mp_type::mm_type::mpmixs_data;
		}
			/* no break statement here */
		case mp_type::mm_type::mpmixs_data:
		{
			const char* bound;
			if (mp_vars.mm_vars.cur_entry->boundary_searcher().search(data, size, bound)) {
				string t = mp_vars.mm_vars.cur_entry->boundary() + "--";
				if (t.compare(0, t.size(), bound) == 0) {
					mp_vars.mm_vars.cur_entry->last_value().append_data(data, bound);
					mp_vars.mm_vars.stop_parsing = true;
					return;
				}
				mp_vars.mm_vars.cur_entry->last_value().append_data(data, bound);
				data = bound + mp_vars.mm_vars.cur_entry->boundary().size();
				data = dataEnd - data;
				mp_vars.mm_vars.state = mp_type::mm_type::mpmixs_header;
				break;
			}
			dataEnd = data;
		}
		} /* switch (mp_vars.mm_vars.state) */
	} /* while (data != dataEnd) */
}

}
} // namespace Http // namespace Fastcgipp_m0sh

#endif


