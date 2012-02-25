//! @file  mosh/fcgi/http/session/session_base.hpp HTTP Session base class
/***************************************************************************
* Copyright (C) 2011-2 m0shbear						   *
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
#include <mosh/fcgi/http/cookie.hpp>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/http/conv/converter.hpp>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/singleton.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

/*! @brief HTTP Environment - POST-agnostic base class
 * @tparam char_type character type
 */
template <typename char_type>
class Session_base {
protected:
	// Without these, readability would suffer hard
	typedef typename std::basic_string<char_type> T_string;
	//! Type alias for cookie entry
	typedef typename form::Entry<char, Cookie> Cookie_v;
	//! Type alias for application/x-www-formurl-encoded entry
	typedef typename form::Entry<char_type> Multi_v;
	//! Type alias for std::map<T_string, Multi_v>
	typedef typename std::map<T_string, Multi_v> Kv;
	//! Type alias for std::map<string. Cookie_v>
	typedef typename std::map<std::string, Cookie_v> Cookie_kv;
public:
	//! Environment variable list
	std::map<std::string, std::string> envs;
	//! GETs
	Kv gets;
	//! (multi-)set of cookies
	Cookie_kv cookies;
	//! Global cookie options
	Cookie cookies_g;
	
	/*! @brief Parses FastCGI parameter data into the data structure
	 *
	 * This function will take the body of a FastCGI parameter record and parse
	 * the data into the data structure. data should equal the first character of
	 * the records body with size being it's content length.
	 *
	 * @param[in] data Pointer to the first byte of parameter data
	 * @param[in] size Size of data in bytes
	 */
	void fill(const uchar* data, size_t size);
		
protected:
	//! Byte buffer
	u_string bbuf;

	Session_base ()  { }

	/*! @name Initialize post data in derived class.
	 */
	//@{
	virtual bool init_ue() = 0;
	virtual bool init_mp(const std::string& mp_bound) = 0;
	//@}

	/*! @name Convert UTF-8 data to Unicode
	 *
	 * @note This function simply returns the contents of
	 * @note ubuf is the char_type template argument is set to @c char.
	 *
	 * In any case, the contents of ubuf are either partially or totally cleared.
	 */
	//@{
	//! Unicode buffer
	u_string ubuf;
	//! Convert the contents of ubuf to Unicode
	std::basic_string<char_type> to_unicode() {
		return to_unicode(this->ubuf);
	}
	//! Convert the contents of ubuf to Unicode
	static std::basic_string<char_type> to_unicode(u_string const&);
	//! Convert the contents of ubuf to Unicode
	static std::basic_string<char_type> to_unicode(u_string&);
	//@}


	/*! @name Decode encoded data using a converter
	 */
	//@{
	//! Encoded data buffer
	std::string ebuf;
	//! Decode the contents of ebuf
	u_string process_encoded_data();
	//! Converter handle
	std::unique_ptr<Converter> conv;
	//@}
	
private:
	/*! @brief Atomically parse a packet of application/x-www-formurl-encoded data without buffering
	 * @note Expects ASCII string data; use @c reinterpret_cast<const char*> if needed
	 * @param[in] data pointer to contents 
	 * @param size length of contents
	 * @param[out] dest ref to destination
	 * @param val_sep value separator
	 */
	void fill_ue_oneshot(const char* data, size_t size, Kv& dest);
};

}

MOSH_FCGI_END

#endif


