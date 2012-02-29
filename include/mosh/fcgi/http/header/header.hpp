//! @file  mosh/fcgi/http/header/header.hpp HTTP Header
/*
 * Copyright (C) 2011-2 m0shbear
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


#ifndef MOSH_FCGI_HTTP_HEADER_HEADER_HPP
#define MOSH_FCGI_HTTP_HEADER_HEADER_HPP

#include <initializer_list>
#include <map>
#include <memory>
#include <ostream>
#include <vector>
#include <string>

#include <mosh/fcgi/http/cookie.hpp>
#include <mosh/fcgi/http/header/helper.hpp>
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
	Header() { }

	//! Helper constructor
	Header(std::shared_ptr<Helper>&& h);

	//! Copy constructor
	Header(Header const& h);

	//! Move constructor
	Header(Header&& h);

	virtual ~Header() { }

	/*! @name Function-call overloads
	 * These overloads are wrappers for the functions described in @c Helper
	 * @throw std::runtime_error if override not found
	 * @sa Helper
	 */
	//@{
	/*! @brief Function-call operator
	 * @param[in] u integer argument
	 * @returns a copy of @c *this with the function's return value appended to it
	 */
	Header operator () (unsigned u) const;

	/*! @brief Function-call operator
	 * @param[in] s string argument
	 * @returns a copy of @c *this with the function's return value appended to it
	 */
	Header operator () (std::string const& s) const;

	/*! @brief Function-call operator
	 * @param[in] u integer argument
	 * @param[in] s string argument
	 * @returns a copy of @c *this with the function's return value appended to it
	 */
	Header operator () (unsigned u, std::string const& s) const;
		
	/*! @brief Function-call operator
	 * @param[in] s string argument
	 * @param[in] u integer argument
	 * @returns a copy of @c *this with the function's return value appended to it
	 */
	Header operator () (std::string const& s, unsigned u) const;
	
	/*! @brief Function-call operator
	 * @param[in] s1 1st string argument
	 * @param[in] s2 2nd string argument
	 * @returns a copy of @c *this with the function's return value appended to it
	 */
	Header operator () (std::string const& s1, std::string const& s2) const;

	/*! @brief Function-call operator
	 * @param[in] u1 1st integer argument
	 * @param[in] u2 2nd integer argument
	 * @returns a copy of @c *this with the function's return value appended to it
	 */
	Header operator () (unsigned u1, unsigned u2) const;

	//@}
	/*! @name Appenders
	 * These overloads append additional headers to the existing one.
	 */
	//@{
	/*! @brief Append a name: value header line
	 *  @param[in] p header pair to append
	 */
	Header& operator += (const std::pair<std::string, std::string>& p);
	
	/*! @brief Append name: value header line(s)
	 *  @param[in] hp {}-list of header pairs to append
	 */
	Header& operator += (std::initializer_list<std::pair<std::string, std::string>> hp);
	
	/*! @brief Append name: value header line(s)
	 *  @param[in] hv vector of header pairs to append
	 */
	Header& operator += (std::vector<std::pair<std::string, std::string>> const& hv);
	//@}
	
	//! String cast operator
	operator std::string () const;

private:
	std::map<std::string, std::vector<std::string>>  headers;
	std::string http_response;

	std::shared_ptr<Helper> helper;
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
Header operator + (const Header& _h, const std::pair<std::string, std::string>& _p);
	
/*! @brief Concatenate a name: value header line
 *  @param[in] _h header
 *  @param[in] _p header pair to concatenate
 */
Header operator + (Header&& _h, const std::pair<std::string, std::string>& _p);
	
/*! @brief Concatenate name: value header line(s)
 *  @param[in] _h header
 *  @param[in] _hp {}-list of header pairs to concatenate
 */
Header operator + (const Header& _h, std::initializer_list<std::pair<std::string, std::string>> _hp);
	
/*! @brief Concatenate name: value header line(s)
 *  @param[in] _h header
 *  @param[in] _hp {}-list of header pairs to concatenate
 */
Header operator + (Header&& _h, std::initializer_list<std::pair<std::string, std::string>> _hp);
//@}

std::ostream& operator << (std::ostream& os, const Header& h);

//! Make pair
std::pair<std::string, std::string> P(std::string&& s1, std::string&& s2);
//! Make pair
std::pair<std::string, std::string> P(std::string const& s1, std::string const& s2);

/*! @name Predefined headers
 */
//@{
/*! @brief @c Content-type header
 * 
 *  Generates a @c Content-type header, with optional charset attribute.
 *
 *  Interface:
 *  @code
 *  (std::string const& ctype)
 *  (std::string const& ctype, std::string const& cset)
 *  ctype -> Content-type
 *  cset -> charset
 *  @endcode
 */
extern const Header content_type;
/*! @brief Redirection header
 * 
 *  Generates a redirection, that is a 3xx Status followed by a Location.
 *
 *  Interface:
 *  @code
 *  (unsigned code, std::string const& loc)
 *  code -> HTTP 3xx code
 *  loc -> Location
 *  @endcode
 *  @throw std::invalid_argument if @c code is not a valid 3xx status.
 */
extern const Header redirect;
/*! @brief Response header
 *
 * Generates a HTTP response line, which must precede Name: value (; attr1=val1 (; attr_n = val_n)?)?
 * header lines for this to be valid CGI output.
 * 
 * Interface:
 * @code
 * (unsigned ver, unsigned code)
 * ver -> (HttpMajorVersion << 8) | HttpMinorVersion
 * code -> HTTP status code
 * @endcode
 */
extern const Header response;
/*! @brief @c Status header
 *
 * Generates a @c Status: line.
 *
 * Interface:
 * @code
 * (unsigned st)
 * st -> HTTP status
 * @endcode
 * @throw std::invalid_argument if @c st is an invalid HTTP status code.
 */
extern const Header status;
/*! @brief @c X-Sendfile header
 *  
 *  Generates a @c X-Sendfile (or equivalent) header line.
 *  
 *  Interface
 *  @code
 *  (std::string const& loc)
 *  (std::string const& loc, unsigned compat)
 *  loc -> location
 *  compat -> compatibility mode:
 *  		{ 0 -> X-Sendfile (normal)
 *  		  1 -> X-Accel-Redirect (nginx)
 *  		}
 *  @endcode
 */
extern const Header x_sendfile;
//@}

}

}

MOSH_FCGI_END

#endif
