//! @file  mosh/fcgi/http/cookie.hpp HTTP Cookie class
/*      
 * Copyright (C) 1996-2004 Stephen F. Booth
 * 		 2007	   Sebastian Diaz
 * 		 2011  	   m0shbear
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA 
 */


#ifndef MOSH_FCGI_HTTP_COOKIE_HPP
#define MOSH_FCGI_HTTP_COOKIE_HPP

#include <initializer_list>
#include <string>
#include <sstream>
#include <utility>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/bits/ci_strcomp.hpp>
#include <mosh/fcgi/bits/t_string.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! HTTP related stuff
namespace http {

/*! @brief HTTP Cookie
 */
class Cookie {
public:
	//! Default constructor
	Cookie()
	{ }
	/*! @brief Create a cookie
	 *  @param[in] name_ cookie name
	 *  @param[in] value_ cookie value
	 */
	Cookie(const std::string& name_, const std::string& value_) noexcept
	: name(name_), value(value_)
	{ }
 
 	/*! @brief Create a cookie ({} form)
 	 *  @param[in] string_args {}-list of string arguments
 	 *  @param[in] int_args {}-list of integral arguments
 	 *  @param[in] bool_args {}-list of boolean arguments
 	 *  @note string_args is of form { key, value *[, comment, domain, path ] }, where * denotes optional
 	 *  @note
 	 *  @note int_args is of form { max_age }
 	 *  @note
 	 *  @note bool_args is of form { secure, *http_only, *removed }
  	 */
 	Cookie(std::initializer_list<std::string> string_args,
 		std::initializer_list<int> int_args = int_args_default,
 		std::initializer_list<bool> bool_args = bool_args_default
 		)
	{
		{
			const std::string* s_arg = string_args.begin();
			switch (string_args.size()) {
			case 5:
				comment = s_arg[2];
				domain = s_arg[3];
				path = s_arg[4];
			case 2:
				name = s_arg[0];
				value = s_arg[1];
				break;
			default:
				throw std::invalid_argument("string_args has an incorrect number of arguments");
			}
		}

		{
			const int* i_arg = int_args.begin();
			switch (int_args.size()) {
			case 1:
				max_age = i_arg[0];
				break;
			default:
				throw std::invalid_argument("i_args has an incorrect number of arguments");
			}
		}
	
		{
			const bool* bool_arg = bool_args.begin();
			switch (bool_args.size()) {
			case 3:
				removed = bool_arg[2];
			case 2:
				http_only = bool_arg[1];
			case 1:
				secure = bool_arg[0];
				break;
			default:
				throw std::invalid_argument("bool_args has an incorrect number of arguments");
			}
		}
	}


	/*! @brief Create a fully-specified cookie
	 *  @param[in] name_ cookie name
	 *  @param[in] value_ cookie value
	 *  @param[in] comment_ any comment associated with this cookie
	 *  @param[in] domain_ a domain for which this cookie is valid
	 *
	 *  @param[in] max_age_ a number of seconds definition the lifetime of this cookie
	 *  @param[in] path_ the subset of URLs in a domain for whih the cookie is valid
	 *  @param[in] secure_ specifies whether this is a secure cookie
	 *  @param[in] http_only_ specified whether HTTP-only scope restriction is in effect
	 */
	Cookie(const std::string& name_, const std::string& value_, const std::string& comment_,
	           const std::string& domain_, int max_age_, const std::string& path_,
		   bool secure_ = false, bool http_only_ = true) noexcept
	: name(name_), value(value_), comment(comment_), max_age(max_age_), domain(domain_), path(path_),
		secure(secure_), http_only(http_only_), removed(false)
	{ }
	//! Copy constructor
	Cookie(const Cookie& cookie) noexcept
	: name(cookie.name), value(cookie.value), comment(cookie.comment), max_age(cookie.max_age),
		domain(cookie.domain), path(cookie.path), secure(cookie.secure), http_only(cookie.http_only),
		removed(cookie.removed)
	{ }
	//! Move constructor
	Cookie(Cookie&& cookie) noexcept
	: name(std::move(cookie.name)), value(std::move(cookie.value)), comment(std::move(cookie.comment)),
		max_age(cookie.max_age), domain(std::move(cookie.domain)), path(std::move(cookie.path)),
		secure(cookie.secure), http_only(cookie.http_only), removed(cookie.removed)
	{ }
	//! Destructor
	virtual ~Cookie()
	{ }
	/*! @brief Create a partially-specified cookie for deletion
	 *  @param[in] name_ cookie name
	 *  @param[in] domain_ a domain for which this cookie is valid
	 *  @param[in] path_ the subset of URLs in a domain for whih the cookie is valid
	 *  @param[in] secure_ specifies whether this is a secure cookie
	 *  @param[in] http_only_ specified whether HTTP-only scope restriction is in effect
	 *  @return a deleted cookie
	 */
	static Cookie deleted(const std::string& name_, const std::string& domain_, 
					const std::string& path_, bool secure_, bool http_only_) noexcept
	{
		Cookie c(name_, "" /* value */, "" /* comment */,  domain_, 0, path_, secure_, http_only_);
		c.removed = true;
		return c;
	}
	//! Compare for equality
	bool operator == (const Cookie& cookie) const {
		return (this == &cookie) ||
			(ci_cmp(name, cookie.name, Cmp_test::eq)
			&& ci_cmp(value, cookie.value, Cmp_test::eq)
			&& ci_cmp(comment, cookie.comment, Cmp_test::eq)
			&& ci_cmp(domain, cookie.domain, Cmp_test::eq)
			&& max_age == cookie.max_age
			&& ci_cmp(path, cookie.path, Cmp_test::eq)
			&& secure == cookie.secure
			&& removed == cookie.removed
			&& http_only == cookie.http_only);
	}

	
	//! Compare for inequality
	bool operator != (const Cookie& cookie) const {
		return !operator==(cookie);
	}

	//! Convert to header line
	operator std::pair<std::string, std::string> () const {
	std::basic_stringstream<std::string> ss;
	ss << name << "=\"" << value << "\"";
	if (!comment.empty()) {
		ss << "; Comment=\"" << comment << "\"";
	}
	if (!domain.empty()) {
		ss << "; Domain=\"" << domain << "\"";
	}
	if (removed) {
		ss << "; Expires=Fri, 01-Jan-1971 01:00:00 GMT";
		ss << "; Max-Age=0";
	} else if (max_age > 0) {
		ss << "; Max-Age=" << max_age;
		ss << "; Expires=" << time_to_string("%a, %d-%b-%Y %H:%M:%S GMT", max_age);
	}
	if (!path.empty()) {
		ss << "; Path=\"" << path << "\"";
	}
	if (secure) {
		ss << "; Secure";
	}
	if (http_only) {
		ss << "; HttpOnly";
	}
	ss << "; Version=\"1\"";
	
	return std::make_pair("Set-Cookie", ss.str());
}


	//! The name of this cookie	
	std::string name;
	//! The value of this cookie
	std::string value;
	//! The comment associated with this cookie
	std::string comment;
	//! The domain for which this cookie is valid
	int max_age;
	//! The subset of URLs in a domain for which this cookie is valid
	std::string domain;
	//! Number of seconds defining the lifetime of this cookie
	std::string path;
	//! Specifies whether this is a secure cookie (i.e. can only be set on a HTTPS connection)
	bool secure;
	/*! @brief Specifies whether to limit the scope of the cookie to HTTP requests (see RFC 6265 (S) 4.1.2.6)
	 *
	 *  Enabled by default for anti-XSS purposes
	 */
	bool http_only;
	//! This cookie's removal state (@c true if deleted, @c false otherwise)
	bool removed;
private:
	static std::initializer_list<int> int_args_default;
	static std::initializer_list<bool> bool_args_default;
};

std::initializer_list<int>  Cookie::int_args_default = { 0 };
std::initializer_list<bool> Cookie::bool_args_default = { false, true, false };

}

MOSH_FCGI_END

#endif
