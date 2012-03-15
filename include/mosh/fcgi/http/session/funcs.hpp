//! @file  mosh/fcgi/http/session/funcs.hpp - Helper functions for http::session
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

#ifndef MOSH_FCGI_HTTP_SESSION_FUNCS_HPP
#define MOSH_FCGI_HTTP_SESSION_FUNCS_HPP

#include <functional>
#include <string>
#include <map>
#include <mosh/fcgi/http/cookie.hpp>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

//! Session-related stuff
namespace session {

/*! @brief Process url-encoded data
 *
 * This extracts the first K=V pair from a buffer and returns the number of bytes consumed.
 *
 * @param[in] data Start of buffer
 * @param[in] size Size of buffer
 * @param[out] k Output key
 * @param[out] v Output value
 * @retval -1 Error due to insufficient length or malformed data
 * @retval 0 No data was used
 * @retval &gt;1 The amount of bytes consumed
 */
ssize_t process_urlencoded_kv(const char* data, size_t size, u_string& k, u_string& v);

/*! @brief Process cookie data
 *
 * This parses the value portion of a Cookie: HTTP header.
 *
 * @param[in] data Start of buffer
 * @param[in] size Size of buffer
 * @param[out] kv The formdata-map for cookie data 
 * @param[out] g The global cookie object
 * @retval -1 Error due to insufficient length or malformed data
 * @retval 0 No data was used
 * @retval &gt;1 The amount of bytes consumed
 */
ssize_t process_cookies(const char* data, size_t size, std::map<std::string, form::Entry<char, Cookie>>& kv, Cookie& g);

/*! @brief Parse a FastCGI parameter
 * 
 * @param[in] p Parameter
 * @param ue_init Functor for url-encoded initialization
 * @param mp_init Functor for multipart-mixed initialization
 * @param do_gets Functor for QUERY_STRING (i.e. GETs) initialization
 * @param do_cookies Functor for cookies initialization
 */
void do_param(std::pair<std::string, std::string> const& p,
		std::function<bool ()> ue_init,
		std::function<bool (std::string)> mp_init,
		std::function<void (const char*, size_t)> do_gets,
		std::function<void (const char*, size_t)> do_cookies);

/*! @brief Parse a multipart/form-data header
 *
 * @param[in] buf Data buffer
 * @param name_func Functor for form item name
 * @param fname_func Functor for form item filename
 * @param cs_func Functor for form item data's charset
 * @param mixed_func Functor for initializing special handling for multipart/mixed
 * @param cte_func Functor for form item data's transfer encoding
 * @param h_func Functor for form item's header map append 
 */
void do_headers(std::string const& buf,
		std::function<void (u_string const&)> name_func,
		std::function<void (u_string const&)> fname_func,
		std::function<void (std::string const&)> cs_func,
		std::function<void (bool)> mixed_func,
		std::function<void (std::string const&)> cte_func,
		std::function<void (std::string const&, std::string const&)> h_func);

}
}

MOSH_FCGI_END

#endif
