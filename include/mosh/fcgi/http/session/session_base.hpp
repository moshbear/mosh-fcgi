// TODO: add complete cookie parsing support

//! @file  mosh/fcgi/http/session/session_base.hpp HTTP Session base class
/***************************************************************************
* Copyright (C) 2011 m0shbear						   *
*									   *
* This file is part of mosh-fcgi.					   *
*									   *
* mosh-fcgi is free software: you can redistribute it and/or modify it	   *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.					   *
*									   *
* mosh-fcgi is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or	   *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public	   *
* License for more details.						   *
*									   *
* You should have received a copy of the GNU Lesser General Public License *
* along with mosh-fcgi.  If not, see <http://www.gnu.org/licenses/>.	   *
****************************************************************************/


#ifndef MOSH_FCGI_HTTP_SESSION_SESSION_BASE_HPP
#define MOSH_FCGI_HTTP_SESSION_SESSION_BASE_HPP

#include <stdexcept>

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/http/conv/converter.hpp>
#include <mosh/fcgi/bits/singleton.hpp>
#include <mosh/fcgi/bits/iconv.hpp>
#include <mosh/fcgi/bits/iconv_gs.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

/*! @brief HTTP Environment - POST-agnostic base class
 * @tparam char_type character type
 */
template <typename char_type>
class Session_base {
protected:
	// Without these, readability would suffer HARD
	typedef typename std::basic_string<char_type> T_string;
	//! Type alias for application/x-www-formurl-encoded entry
	typedef typename form::Entry<char_type> Multi_v;
	//! Type alias for std::map<T_string, Multi_v>
	typedef typename std::map<T_string, Multi_v> Kv;
private:
	struct regex_cache;
public:
	//! Environment variable list
	std::map<std::string, std::string> envs;
	//! GETs
	Kv gets;
	//! (multi-)set of cookies
	Kv cookies;
	
	/*! @brief Parses FastCGI parameter data into the data structure
	 *
	 * This function will take the body of a FastCGI parameter record and parse
	 * the data into the data structure. data should equal the first character of
	 * the records body with size being it's content length.
	 *
	 * @param[in] data Pointer to the first byte of parameter data
	 * @param[in] size Size of data in bytes
	 */
	void fill(const char* data, size_t size);
		
protected:
	//! A buffer for internal use by any of fill*
	std::string xbuf;
	//! Buffer for to_unicode()
	std::string ubuf;

	Session_base () { }

	/*! @name Initialize post data in derived class.
	 */
	//@{
	virtual bool init_ue() = 0;
	virtual bool init_mp(const std::string& mp_bound) = 0;
	//@}
	
	//! Get the charset setter
	Iconv::_s_ic_state charset() { return Iconv::_s_ic_state_u(ic); }

	/*! @name Convert input to unicode
	 */
	//@{
	std::basic_string<char_type> to_unicode();
	//! Iconv state handle
	std::unique_ptr<Iconv::IC_state> ic;
	//@}


	/*! @name Decode encoded data using a converter
	 */
	//@{
	std::string process_encoded_data();
	//! Converter handle
	std::unique_ptr<Converter> conv;
	//@}
	

	/*! @brief Get an instance of the regex cache.
	 *
	 * On first invocation, the regex_cache constructor is called,
	 * which ensures that we have to compile the regexes only once.
	 * With the singleton method, we defer regex compilation to when
	 * multipart parsing actually begins.
	 * @returns an instance of the regex cache
	 */
	regex_cache& rc() {
		return S_rc.instance();
	}
private:
	/*! @brief Atomically parse a packet of application/x-www-formurl-encoded data without buffering
	 * @param[in] data pointer to contents 
	 * @param size length of contents
	 * @param[out] dest ref to destination
	 * @param val_sep value separator
	 */
	void fill_ue_oneshot(const char* data, size_t size, Kv& dest, char val_sep = '&');
	
	//! Http::Environment<T,V> regex cache
	static Singleton<regex_cache> S_rc;
};

}

MOSH_FCGI_END

#endif


