//! @file mosh/fcgi/http/session/session.tcc Template implementations for Session
/***************************************************************************
* Copyright (C) 2011-2 m0shbear                                            *
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


#ifndef MOSH_FCGI_HTTP_SESSION_SESSION_TCC
#define MOSH_FCGI_HTTP_SESSION_SESSION_TCC

#include <algorithm>
#include <functional>
#include <memory>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/xpressive/xpressive.hpp>
#include <mosh/fcgi/bits/boyer_moore.hpp>
#include <mosh/fcgi/http/conv/converter.hpp>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/http/session/session_base.hpp>
#include <mosh/fcgi/http/session/session.hpp>
#include <mosh/fcgi/http/session/funcs.hpp>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

namespace xpr = boost::xpressive;
namespace stdph = std::placeholders;


template <typename ct, typename pt>
struct Session<ct, pt>::Ue_type {
	//! Pointer to last entry
	MP_entry* cur_entry;
	//! At which stage of KV parsing are we?
	enum class State { name, value } state;
	Ue_type() : state(State::name) { }
	virtual ~Ue_type() { }
};


template <typename ct, typename pt>
struct Session<ct, pt>::Mp_type {
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
void Session<ct, pt>::fill_post(const uchar* data, size_t size) {
	if (this->multipart)
		this->fill_mp(data, size);
	else
		this->fill_ue(sign_cast<const char*>(data), size);
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

			this->ebuf.append(data, _eoh);
			data = _eoh + 1;
			this->ubuf += this->process_encoded_data();
			if (_eoh == data_end)
				return;
			
			MP_entry cur_fe(this->to_unicode());
			// Prepare pointers for state::data mode
			this->posts[cur_fe.name] << std::move(cur_fe);
			this->ue_vars->cur_entry = &(this->posts[cur_fe.name].last_value());
			this->ue_vars->state = Ue_type::State::value;
			// Clear ebuf and ubuf to avoid unintented conversion results 
			this->ebuf.clear();
			this->ubuf.clear();

			data = _eoh + 1;
		}
		case Ue_type::State::value:
		{
			const char* _eov = std::find(data, data_end, '&');
			this->ebuf.append(data, _eov);
			this->ubuf += this->process_encoded_data();
			this->ue_vars->cur_entry->append_text(this->to_unicode());
			if (_eov != data_end) {
				this->ue_vars->state = Ue_type::State::name;
				data = _eov + 1;
				// Clear ebuf and ubuf to avoid unintented conversion results 
				this->ebuf.clear();
				this->ubuf.clear();
			} else
				return;
		}	
		} /* switch (ue_vars->state) */
	} /* while (data != data_end) */
}

template <typename ct, typename pt>
void Session<ct, pt>::fill_mp(const uchar* data, size_t size) {
	if (this->mp_vars->stop_parsing)
		return;
	const uchar* const data_end = data + size;	
	while (data != data_end) {
		switch (this->mp_vars->state) {
		case Mp_type::State::header:
		{
			const char* _clcl = "\r\n\r\n"; // 4*n ~= m*n; don't need boyer-moore
			const uchar* _eoh = std::search(data, data_end, _clcl, _clcl + 4);
			this->ebuf.append(sign_cast<const char*>(data), sign_cast<const char*>(_eoh));
			if (_eoh == data_end)
				return;

			MP_entry cur_entry;
			session::do_headers(this->ebuf,
				[&] (u_string const& a1) { cur_entry.name = this->to_unicode(a1); }, // name_func 
				[&] (u_string const& a1) { cur_entry.filename = this->to_unicode(a1); }, // fname_func
				[&] (std::string const& a1) { cur_entry.charset = a1; }, // cs_func 
				[&] (bool a1) {
					this->mp_vars->mixed = a1;
					if (a1)
						this->mp_vars->mm_vars.stop_parsing = false;
				},
				[&] (std::string const& a1) { cur_entry.ct_encoding = a1; },
				[&] (std::string const& a1, std::string const& a2) {
					if (!a1.compare("Content-Type"))
						cur_entry.content_type = a2;
					else
						cur_entry.add_header(a1, a2);
				}
			);
			this->ebuf.clear();
			data = _eoh + 4;
			if (this->mp_vars->mixed) {
				using namespace xpr;
				// prepare the cur_entry
				MP_mixed_entry cur_mm(std::move(cur_entry));
				auto _ct = cur_mm.headers.find("Content-Type");
				if (_ct != cur_mm.headers.end()) {
					std::string bound = session::boundary_from_ct(_ct->second);
					if (!bound.empty())
						cur_mm.set_boundary(std::move(bound));
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

			const uchar* bound;
			bool found = this->mp_vars->s_bound.search(data, (data_end - data), bound);
			if (!found) {
				bound = data_end;
			}
			if (this->mp_vars->mixed) {
				this->fill_mm(data, bound - data);
			} else {
				/* Enable this if you're going to be using iconv
				if (!cur.charset.empty()) {
					this->charset() = cur.charset;
				}
				*/
				if (!cur.ct_encoding.empty()) {
					this->ebuf.append(sign_cast<const char*>(data), sign_cast<const char*>(bound));
					u_string dd = this->process_encoded_data();
					if (cur.is_file()) {
						cur.append_binary(dd.data(), dd.data() + dd.size());
					} else {
						this->ubuf += dd;
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
				&& t.compare(0, t.size(), sign_cast<const char*>(bound)) == 0) {
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
void Session<ct, pt>::fill_mm(const uchar* data, size_t size) {
	if (this->mp_vars->mm_vars.stop_parsing)
		return;
	const uchar* const data_end = data + size;	
	while (data != data_end) {
		switch (this->mp_vars->mm_vars.state) {
		case Mp_type::Mm_type::State::header:
		{
			const uchar* _clcl = sign_cast<const uchar*>("\r\n\r\n"); // 4*n ~= m*n; don't need boyer-moore
			const uchar* _eoh = std::search(data, data_end, _clcl, _clcl + 4);
			this->ebuf.append(sign_cast<const char*>(data), sign_cast<const char*>(_eoh));
			// No terminator found; keep on buffering
			if (_eoh == data_end)
				return;
			
			MP_entry cur_mp;
			
			session::do_headers(this->ebuf,
				[&] (u_string const& a1) { cur_mp.name = this->to_unicode(a1); }, // name_func
				[&] (u_string const& a1) { cur_mp.filename = this->to_unicode(a1); }, // fname_func
				[&] (std::string const& a1) { cur_mp.charset = a1; }, // cs_func
				[ ] (bool) { }, // dummy
				[&] (std::string const& a1) { cur_mp.ct_encoding = a1; },
				[&] (std::string const& a1, std::string const& a2) {
					if (!a1.compare("Content-Type"))
						cur_mp.content_type = a2;
					else
						cur_mp.add_header(a1, a2);
				}
			);
			
			this->ebuf.clear();
			data = _eoh + 4;
			
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
			
			const uchar* bound;
			bool found = cur.boundary_searcher().search(data, (data_end - data), bound);
			if (!found)
				bound = data_end;
			/* Enable this if you're going to be using iconv
			if (!mp.charset.empty()) {
				this->charset() = mp.charset;
			}	
			*/
			if (!mp.ct_encoding.empty()) {
				this->ebuf.append(sign_cast<const char*>(data), sign_cast<const char*>(bound));
				u_string dd = this->process_encoded_data();
				if (mp.is_file()) {
					mp.append_binary(dd.data(), dd.data() + dd.size());
				} else {
					this->ubuf += dd;
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
				&& t.compare(0, t.size(), sign_cast<const char*>(bound)) == 0) {
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


} // namespace http

MOSH_FCGI_END

#endif


