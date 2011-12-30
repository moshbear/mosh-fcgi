//! @file  mosh/fcgi/http/cookie.hpp HTTP Cookie class
/*                              
 * Copyright (C) 2011  	   m0shbear
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
#include <map>
#include <string>
#include <utility>
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
	{
		reserve();
		kvs.insert(std::make_pair(name_, value_);
	}

	/*! @brief Create a cookie from a list of KV attributes
	 * @param[in] kvs List of attributes
	 * @throws std::invalid_argument if kvs contains reserved names
	 * 		(i.e. Comment, Domain, Expires, Max-Age, Path, Version, Secure, HttpOnly)
	 */
	Cookie(std::initializer_list<std::pair<std::string, std::string>> kvs)
	{
		for (std::pair const& arg : kvs) {
			
			kvs.insert(arg);
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
	bool operator == (const Cookie& cookie) const;
	
	//! Compare for inequality
	bool operator != (const Cookie& cookie) const {
		return !operator==(cookie);
	}

	//! Convert to header line
	operator std::pair<std::string, std::string> () const;

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
	std::map<std::string, std::string, ci_cmp_wrapper<char, Cmp_test::lt>> kvs;

	//! Reserve certain KV attributes to avoid mistakes
	void reserve() {
		kvs["Comment"] = kvs["$Comment"] = "";
		kvs["Domain"] = kvs["$Domain"] = "";
		kvs["Expires"] = kvs["$Expires"] = "";
		kvs["Max-Age"] = kvs["Max-Age"] = "";
		kvs["Path"] = kvs["$Path"] = "";
		kvs["Version"] = kvs["$Version"] = "";
	 * 		(i.e. Comment, Domain, Expires, Max-Age, Path, Version, Secure, HttpOnly)
			
		
};

}

MOSH_FCGI_END

#endif
