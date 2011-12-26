//! @file mosh/fcgi/http/session/session.hpp HTTP Session
/***************************************************************************
* Copyright (C) 2011 m0shbear						   *
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

#ifndef MOSH_FCGI_HTTP_SESSION_SESSION_HPP
#define MOSH_FCGI_HTTP_SESSION_SESSION_HPP

#include <map>
#include <memory>
#include <string>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/http/session/session_base.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

/*! @brief HTTP Environment
 * @tparam char_type Character type to use for strings
 * @tparam post_val_type Value type for POST entries
 */
template <typename char_type, typename post_val_type = std::basic_string<char_type>>
class Session : public virtual Session_base<char_type>
{
protected:
	//! Type alias for multipart/form-data entry
	typedef typename form::MP_entry<char_type, post_val_type> MP_entry;
	//! Type alias for a list of multipart/form-data entries sharing a common name
	typedef typename form::Entry<char_type, MP_entry> MP_input;
	//! Type alias for multipart/mixed entry
	typedef typename form::MP_mixed_entry<char_type, post_val_type> MP_mixed_entry;
	//! Type alias for a list of multipart/mixed entries sharing a common name
	typedef typename form::Entry<char_type, MP_mixed_entry> MP_mixed_input;
public:
	//! POSTs
	std::map<typename Session_base<char_type>::T_string, MP_input> posts;
	// ! multipart/mixed POSTs
	std::map<typename Session_base<char_type>::T_string, MP_mixed_input> mm_posts;
private:

	//! @c true if envs["CONTENT_TYPE"] contains multipart/form-data
	bool multipart;

	/*! @name Application/x-www-formurl-encoded specifics
	 */
	//@{
	struct Ue_type;
	struct Ue_regex_cache;
	std::unique_ptr<Ue_type> ue_vars;
	//@}
	
	/*! @name Multipart/form-data specifics
	 */
	//@{
	struct Mp_type;
	struct Mp_regex_cache;
	std::unique_ptr<Mp_type> mp_vars;
	//@}
	
public:
	//! Default constructor	
	Session() { }
	
	virtual ~Session() { }

	/*! @brief Appends the contents of an IN record to the POST buffer
	 * @param[in] data Pointer to the first byte of post data
	 * @param[in] size Size of data in bytes
	 */
	void fill_post(const char* data, size_t size);

protected:
	//! Prepare this % Session for @c application/x-www-formurl-encoded POSTDATA
	bool init_ue();
	/*! @brief Prepare this % Session for @c multipart/form-data POSTDATA
	 *  @param[in] mp_bound The value of attribute boundary in env[CONTENT_TYPE]
	 */
	bool init_mp(const std::string& mp_bound);
private:
	/*! @name POSTDATA fill helpers
	 */
	//@{
	/*! @brief Fill an @c application/x-www-formurl-encoded entry
	 * @param[in] data pointer to data
	 * @param size length of data
	 */
	void fill_ue(const char* data, size_t size);
	/*! @brief Parse a packet of @c multipart/form-data data.
	 * @param[in] data pointer to data
	 * @param size length of data
	 */
	void fill_mp(const char* data, size_t size);

	/*! @brief Fill a multipart/mixed entry
	 * @remark This function is marked private because it relies on preconditions which
	 * 	 are only guaranteed if called from fill_mp(const char*, size_t).
	 * @param[in] data pointer to data
	 * @param size length of data
	 */
	void fill_mm(const char* data, size_t size);
	//@}

	/*! @brief Convert a RFC 822 header into a std::map
	 * This reads the RFC 822 header in a buffer and returns the result in std::map form.
	 * Words (in the key portion) are capitalized across headers.
	 * @param[in] buf the buffer
	 * @returns the contents of the buffer in a std::map format
	 */
	std::map<std::string, std::string> mp_read_header(const std::string& buf) const;
	 
};

} // namespace http

MOSH_FCGI_END

#endif


