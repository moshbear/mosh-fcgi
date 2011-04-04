//! \file http/http.hpp Defines elements of the HTTP protocol
/***************************************************************************
* Copyright (C) 2011 m0shbear						   *
* 		2007 Eddie					  	   *
*									   *
* This file is part of fastcgi++.					   *
*									   *
* fastcgi++ is free software: you can redistribute it and/or modify it	   *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.					   *
*									   *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or	   *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public	   *
* License for more details.						   *
*									   *
* You should have received a copy of the GNU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.	   *
****************************************************************************/


#ifndef SESSION_BASE_HPP
#define SESSION_BASE_HPP

#include <stdexcept>

#include <map>
#include <string>
#include <vector>
#include <fastcgipp-mosh/protocol.hpp>
#include <fastcgipp-mosh/unicode.hpp>
#include <fastcgipp-mosh/http/cookie.hpp>
#include <fastcgipp-mosh/http/form.hpp>
#include <fastcgipp-mosh/bits/boyer_moore.hpp>
#include <fastcgipp-mosh/bits/singleton.hpp>
#ifdef FASTCGIPP_USE_HYBRID_VECTOR
#define VEC_OPS__HV
#include <hybrid_vector>
#endif
#ifdef FASTCGIPP_USE_STXXL
#define VEC_OPS__STXXL
#include <stxxl/vector>
#endif
#define VEC_OPS__BEGIN namespace VecOps {
#define VEC_OPS__END }
#include <fastcgipp-mosh/bits/vec_ops.hpp>


namespace Fastcgipp_m0sh
{

namespace Http
{

struct regex_cache;
struct ue_regex_cache;
struct mp_regex_cache;

//! Data structure of HTTP session data
/*!
 * @tparam char_type Character type to use for strings
 * @tparam post_vec_type Vector type for multipart filedata
 * @tparam data_vec_type Vector type for data_buffer (only applicable when ROLE==ROLE_FILTER)
 */
template <class char_type, class post_vec_type = std::vector<char>, class data_vec_type = post_vec_type >
struct Environment
{
private:
	// Without these, readability would suffer HARD
	typedef typename std::basic_string<char_type> TString;
	typedef typename Form::Entry<char_type> Form_entry;
	typedef typename Form::MP_entry<char_type, post_vec_type> MP_entry;
	typedef typename Form::Entry<char_type, MP_entry> MP_input;
	typedef typename Form::MP_mixed_entry<char_type, post_vec_type> MP_mixed_entry;
	typedef typename Form::Entry<char_type, MP_mixed_entry> MP_mixed_input;
public:
	//! Environment variable list
	std::map<std::string, std::string> environment;
	//! GET data
	std::map<TString, Form_entry> gets;
	//! Entry POSTDATA (no files)
	std::map<TString, Form_entry> posts;
	//! Multipart POSTDATA
	std::map<TString, MP_input> multipart_posts;
	// ! Multipart/mixed POSTDATA
	std::map<TString, MP_mixed_input> multipart_mixed_posts;
	//! FCGI_DATA (for ROLE==ROLE_FILTER)
	data_vec_type data_buffer;
private:
	//! a buffer for internal use by any of fill*
	std::string buf;

	//! @c true if data is multipart
	bool multipart;

	//! Http::Environment<T,V> regex cache
	static Singleton<regex_cache> S_rc;
	/*! @brief Get an instance of the regex cache.
	 * On first invocation, the regex_cache constructor is called,
	 * which ensures that we have to compile the regexes only once.
	 * With the singleton method, we defer regex compilation to when
	 * multipart parsing actually begins instead of in the Http::Environment
	 * constructor.
	 * @returns an instance of the regex cache
	 */
	regex_cache& rc() {
		return S_rc.instance();
	}
	
	//! application/x-www-formurl-encoded specifics
	struct ue_type {
		/*! @brief Get an instance of the urlencoded's regex cache.
		 * On first invocation, the ue_regex_cache constructor is called,
		 * which ensures that we have to compile the regexes only once.
		 * With the singleton method, we defer regex compilation to when
		 * multipart parsing actually begins instead of in the Http::Environment
		 * constructor.
		 * @returns an instance of the urlencoded's regex cache
		 */
		ue_regex_cache& rc() {
			return S_rc.instance();
		}
		//! pointer to last entry
		Form_entry* cur_entry;
		//! At which stage of KV parsing are we?
		enum { ues_name, ues_value } state;
		//! Reinitialize state
		void reset() {
			state = ues_name;
		}
	private:
		//! regex cache for ue_vars
		static Singleton<ue_regex_cache> S_rc;
	} ue_vars;
	
	//! multipart/form-data specifics
	struct mp_type {
		/*! @brief Get an instance of the multipart's regex cache.
		 * On first invocation, the mp_regex_cache constructor is called,
		 * which ensures that we have to compile the regexes only once.
		 * With the singleton method, we defer regex compilation to when
		 * multipart parsing actually begins instead of in the Http::Environment
		 * constructor.
		 * @returns an instance of the multipart's regex cache
		 */
		regex_cache& rc() {
			return S_rc.instance();
		}
		//! stage of multipart/form-data parsing
		enum { mps_header, mps_data } state;
		//! POST boundary for multitype objects
		std::string boundary; // --$boundary (no newline!)
		Boyer_moore_searcher s_bound;
		//! pointer to last entry
		MP_entry* cur_entry;
		//! @c true if data needs to be passed to parseMultipartMixed
		bool mixed;
		
		//! Reinitialize state
		void reset() {
			state = mps_header;
			mm_vars.reset();
		}

		//! multipart/mixed specifics
		struct mm_type {
			//! stage of multipart/mixed parsing
			enum { mpmixs_header, mpmixs_data } state;
			MP_mixed_entry* cur_entry;
			//! @c true if --(cur_entry->boundary)-- was found
			bool stop_parsing;
			//! Reinitialize state
			void reset() {
				state = mpmixs_header;
			}
		} mm_vars;

		//! @c true if --boundary-- was found
		bool stop_parsing;
	private:
		//! regex cache for mp_vars
		static Singleton<mp_regex_cache> S_rc;
	} mp_vars;
public:
	
	Environment() {
		ue_vars.reset();
		mp_vars.reset(); // calls mp_vars.mm_vars.reset()
	}

	/* @brief Parses FastCGI parameter data into the data structure
	 * This function will take the body of a FastCGI parameter record and parse
	 * the data into the data structure. data should equal the first character of
	 * the records body with size being it's content length.
	 *
	 * @param[in] data Pointer to the first byte of parameter data
	 * @param[in] size Size of data in bytes
	 */
	bool fill(const char* data, size_t size);
	
	/*! @brief Appends the contents of an IN record to the POST buffer
	 * @param[in] data Pointer to the first byte of post data
	 * @param[in] size Size of data in bytes
	 */
	void fillPost(const char* data, size_t size);

	/*! @brief Appends the contents of a DATA record to the DATA buffer
	 * Only applicable when FCGI_ROLE==FILTER
	 * @param[in] data Pointer to the first byte of data
	 * @param[in] size Size of data in bytes
	 */
	void fillData(const char* data, size_t size) {
		VecOps::vec_ops<data_vec_type>::append(data_buffer, data, data + size);
	}

protected:
	/*! @brief Parse QUERY_STRING
	 * @param[in] data pointer to contents of QUERY_STRING
	 * @param size length of QUERY_STRING
	 */
	void fillGets(const char* data, size_t size);
	/*! @brief Parse a packet of application/x-www-formurl-encoded data.
	 * @param[in] data pointer to data
	 * @param size length of data
	 */
	void fillUrlEncoded(const char* data, size_t size);
	/*! @brief Parse a packet of multipart/form-data data.
	 * @param[in] data pointer to data
	 * @param size length of data
	 */
	void fillMultipart(const char* data, size_t size);
	/*! @brief Convert a MIME header into a std::map
	 * This reads the MIME header in a buffer and returns the result in std::map form.
	 * Words (in the key portion) are capitalized across headers.
	 * @param[in] buf the buffer
	 * @returns the contents of the buffer in a std::map format
	 */
	std::map<std::string, std::string> mp_read_header(const std::string& buf) const;
private:
	/*! @brief Fill a multipart/mixed entry
	 * @remark This function is marked private because it relies on preconditions which
	 * 	 are only guaranteed if called from fillMultipart(const char*, size_t).
	 * @param[in] data pointer to data
	 * @param size length of data
	 */
	void fillMultipartMixed(const char* data, size_t size);
};

}
} // namespace Http // namespace Fastcgipp_m0sh

#endif


