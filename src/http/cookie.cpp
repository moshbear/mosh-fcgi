//! @file http/cookie.cpp HTTP Cookie class
/*                              
 * Copyright (C) 1996-2004 Stephen F. Booth
 * 		 2007	   Sebastian Diaz
 * 		 2011  	   m0shbear
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

#include <initializer_list>
#include <string>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <mosh/fcgi/http/cookie.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/bits/ci_strcomp.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! HTTP related stuff
namespace http {

/*! @brief Create a cookie ({} form)
 *  @param[in] string_args {}-list of string arguments
 *  @param[in] ulong_args {}-list of integral arguments
 *  @param[in} bool_args {}-list of boolean arguments
 *  @note string_args is of form { key, value *[, comment, domain, path ] }, where * denotes optional
 *  @note
 *  @note int_args is of form *{ max_age }
 *  @note
 *  @note bool_args is of form *{ secure, *removed }
 */
Cookie::Cookie(std::initializer_list<std::string> string_args,
	std::initializer_list<int> int_args,
	std::initializer_list<bool> bool_args)
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

//! Compare for equality
bool Cookie::operator == (const Cookie& cookie) const {
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

//! Convert to string
Cookie::operator std::pair<std::string, std::string> () const {
	std::stringstream ss;
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

std::initializer_list<int>  Cookie::int_args_default = { 0 };
std::initializer_list<bool> Cookie::bool_args_default = { false, true, false };

} // http

MOSH_FCGI_END

