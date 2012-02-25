//! @file  mosh/fcgi/http/header.hpp HTTP Header
/*
 * Copyright (C) 2011 m0shbear
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


#ifndef MOSH_FCGI_HTTP_HEADER_HPP
#define MOSH_FCGI_HTTP_HEADER_HPP

#include <initializer_list>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <string>

#include <mosh/fcgi/http/cookie.hpp>
#include <mosh/fcgi/http/helpers/helper.hpp>
#include <mosh/fcgi/http/helpers/content_type.hpp>
#include <mosh/fcgi/http/helpers/redirect.hpp>
#include <mosh/fcgi/http/helpers/response.hpp>
#include <mosh/fcgi/http/helpers/status.hpp>
#include <mosh/fcgi/http/helpers/x_sendfile.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

//! HTTP header
namespace header {

//! HTTP header generator
class Header {
public:
	std::map<std::string, std::vector<Cookie>> cookies;

	//! Default constructor
	Header()
	{ }

	//! Helper constructor
	Header(const Helper& h)
	: helper(h)
	{ }

	//! Copy constructor
	Header(const Header& h)
	: helper(h.helper), headers(h.headers), http_response(h.http_response)
	{ }

	//! Move constructor
	Header(Header&& h)
	: helper(h.helper), headers(std::move(h.headers)), http_response(std::move(h.http_response))
	{ }

	virtual ~Header()
	{ }

	/*! @name Function-call overloads
	 * These overloads are wrappers for the functions described in @c Helper
	 * @sa Helper
	 */
	//@{
	/*! @brief Function-call operator
	 * The behavior of this function depends on the value of @c helper.do_u.
	 * If well-defined, the result of do_u is appended to the internal buffer.
	 * Otherwise, std::invalid_argument is thrown.
	 * @param[in] u argument to be passed to do_u
	 * @returns a copy of @c *this with the function's return value appended to it
	 * @throws std::invalid_argument if helper.do_u is null, that is, undefined
	 */
	Header operator () (unsigned u) const {
		Header h(*this);
		if (helper.do_u) {
			h += helper.do_u(u);
			return h;
		} else
			throw std::invalid_argument("operator()(unsigned) is undefined");
	}

	/*! @brief Function-call operator
	 * The behavior of this function depends on the value of @c helper.do_u.
	 * If well-defined, the result of do_u is appended to the internal buffer.
	 * Otherwise, std::invalid_argument is thrown.
	 * @param[in] s string argument
	 * @returns a copy of @c *this with the function's return value appended to it
	 * @throws std::invalid_argument if helper.do_u is null, that is, undefined
	 */
	Header operator () (const std::string& s) const {
		Header h(*this);
		if (helper.do_s) {
			h += helper.do_s(s);
			return h;
		} else
			throw std::invalid_argument("operator()(string) is undefined");
	}

	/*! @brief Function-call operator
	 * The behavior of this function depends on the value of @c helper.do_u_s.
	 * If well-defined, the result of do_u_s is appended to the internal buffer.
	 * Otherwise, std::invalid_argument is thrown.
	 * @param[in] u 1st argument to be passed to do_u_s
	 * @param[in] s 2nd argument to be passed to do_u_s
	 * @returns a copy of @c *this with the function's return value appended to it
	 * @throws std::invalid_argument if helper.do_u_s is null, that is, undefined
	 */
	Header operator () (unsigned u, const std::string& s) const {
		Header h(*this);
		if (helper.do_u_s) {
			h += helper.do_u_s(u, s);
			return h;
		} else
			throw std::invalid_argument("operator()(unsigned, string) is undefined");
	}
		
	/*! @brief Function-call operator
	 * The behavior of this function depends on the value of @c helper.do_s_u.
	 * If well-defined, the result of do_s_u is appended to the internal buffer.
	 * Otherwise, std::invalid_argument is thrown.
	 * @param[in] s 1st argument to be passed to do_s_u
	 * @param[in] u 2nd argument to be passed to do_s_u
	 * @returns a copy of @c *this with the function's return value appended to it
	 * @throws std::invalid_argument if helper.do_s_u is null, that is, undefined
	 */
	Header operator () (const std::string& s, unsigned u) const {
		Header h(*this);
		if (helper.do_s_u) {
			h += helper.do_s_u(s, u);
			return h;
		} else
			throw std::invalid_argument("operator()(string, unsigned) is undefined");
	}
	
	/*! @brief Function-call operator
	 * The behavior of this function depends on the value of @c helper.do_s_s.
	 * If well-defined, the result of do_s_s is appended to the internal buffer.
	 * Otherwise, std::invalid_argument is thrown.
	 * @param[in] s1 1st argument to be passed to do_s_s
	 * @param[in] s2 2nd argument to be passed to do_s_s
	 * @returns a copy of @c *this with the function's return value appended to it
	 * @throws std::invalid_argument if helper.do_s_s is null, that is, undefined
	 */
	Header operator () (const std::string& s1, const std::string& s2) const {
		Header h(*this);
		if (helper.do_s_s) {
			h += helper.do_s_s(s1, s2);
			return h;
		} else
			throw std::invalid_argument("operator()(string, string) is undefined");
	}
	//@}
	/*! @name Appenders
	 * These overloads append additional headers to the existing one.
	 */
	//@{
	/*! @brief Append a name: value header line
	 *  @param[in] p header pair to append
	 */
	Header& operator += (const std::pair<std::string, std::string>& p) {
		if (p.first == "@@http_response@@") {
			http_response = p.second;
		} else {
			headers[p.first].push_back(p.second);
		}
		return *this;
	}
	
	/*! @brief Append name: value header line(s)
	 *  @param[in] hp {}-list of header pairs to append
	 */
	Header& operator += (std::initializer_list<std::pair<std::string, std::string>> hp) {
		for (const auto& p : hp) {
			operator += (p);
		}
		return *this;
	}
	
	/*! @brief Append name: value header line(s)
	 *  @param[in] hv vector of header pairs to append
	 */
	Header& operator += (std::vector<std::pair<std::string, std::string>> const& hv) {
		for (const auto& p : hv) {
			operator += (p);
		}
		return *this;
	}
	//@}
	
	//! String cast operator
	operator std::string () const {
		std::stringstream ss;
		if (!http_response.empty()) {
			ss << http_response << "\r\n";
		}
		for (const auto& h_k : headers) {
			for (const auto& h_v: h_k.second)  {
				ss << h_k.first << ": " << h_v << "\r\n";
			}
		}
		ss << "\r\n";
		return ss.str();
	}

	Helper helper;
private:
	std::map<std::string, std::vector<std::string>>  headers;
	std::string http_response;
};

/*! @name Concatenators
  * These overloads create new headers equivalent to 
  * additional headers being appended to an existing one.
  */
//@{

/*! @brief Concatenate a name: value header line
 *  @param[in] _h header
 *  @param[in] _p header pair to concatenate
 */
Header operator + (const Header& _h, const std::pair<std::string, std::string>& _p) {
	Header h(_h);
	h += _p;
	return h;
}
	
/*! @brief Concatenate a name: value header line
 *  @param[in] _h header
 *  @param[in] _p header pair to concatenate
 */
Header operator + (Header&& _h, const std::pair<std::string, std::string>& _p) {
	Header h(_h);
	h += _p;
	return std::move(h);
}
	
/*! @brief Concatenate name: value header line(s)
 *  @param[in] _h header
 *  @param[in] _hp {}-list of header pairs to concatenate
 */
Header operator + (const Header& _h, std::initializer_list<std::pair<std::string, std::string>> _hp) {
	Header h(_h);
	h += _hp;
	return h;
}
	
/*! @brief Concatenate name: value header line(s)
 *  @param[in] _h header
 *  @param[in] _hp {}-list of header pairs to concatenate
 */
Header operator + (Header&& _h, std::initializer_list<std::pair<std::string, std::string>> _hp) {
	Header h(std::move(_h));
	h += _hp;
	return std::move(h);
}	
//@}

std::ostream& operator << (std::ostream& os, const Header& h) {
	os << static_cast<std::string>(h);
	return os;
}

//! Make pair
std::pair<std::string, std::string> P(std::string&& s1, std::string&& s2) {
	return std::make_pair(std::move(s1), std::move(s2));
}
//! Make pair
std::pair<std::string, std::string> P(const std::string& s1, const std::string& s2) {
	return std::make_pair(s1, s2);
}

/*! @name Predefined headers
 */
//@{
const Header content_type (helpers::content_type::helper());
const Header redirect (helpers::redirect::helper());
const Header response (helpers::response::helper());
const Header status (helpers::status::helper());
const Header x_sendfile (helpers::xsendfile::helper());
//@}

}

}

MOSH_FCGI_END

#endif
