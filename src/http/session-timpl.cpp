//! @file http/session-timpl.cpp Template instantiations of http/session/*.tcc
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

#include <string>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/http/session/session_base.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/utf8.hpp>
#include <src/namespace.hpp>
	

MOSH_FCGI_BEGIN

namespace http {

/* The following conditions must be met for an instantiation of Session_base<T>::to_unicode():
 * 1) It must clear ubuf, partially or completely
 * 2) It must return std::basic_string<T>
 * 3) If it makes use of any (new) state buffers, e.g. iconv state, it must not limit the
 * 	possible instatiations. That is, if the original code compiles with T = char, wchar_t, uchar, and int,
 * 	then the new code must also compile with T = char, wchar_t, uchar, and int.
 */
template <>
std::string Session_base<char>::to_unicode(u_string& u) {
	std::string _s(sign_cast<const char*>(u.data()), sign_cast<const char*>(u.data()) + u.size());
	u.clear();
	return _s;
}

template <>
u_string Session_base<uchar>::to_unicode(u_string& u) {
	u_string _s;
	_s.swap(u); // std::move would also work
	return _s;
}

template <>
std::wstring Session_base<wchar_t>::to_unicode(u_string& u) {
	std::wstring ret;
	wchar_t* t_next;
	ret.resize(u.size());
	const uchar* f_next;
	
	SRC::utf8_in(u.data(), u.data() + u.size(), f_next, &(*ret.begin()), &(*ret.end()), t_next);
	u.erase(0, f_next - u.data());
	ret.resize(t_next - ret.data());
	return ret;
}

// Const versions
template <>
std::string Session_base<char>::to_unicode(u_string const& u) {
	std::string _s(sign_cast<const char*>(u.data()), sign_cast<const char*>(u.data()) + u.size());
	return _s;
}

template <>
u_string Session_base<uchar>::to_unicode(u_string const& u) {
	u_string _s(u);
	return _s;
}

template <>
std::wstring Session_base<wchar_t>::to_unicode(u_string const& u) {
	std::wstring ret;
	wchar_t* t_next;
	ret.resize(u.size());
	const uchar* f_next;
	
	SRC::utf8_in(u.data(), u.data() + u.size(), f_next, &(*ret.begin()), &(*ret.end()), t_next);
	ret.resize(t_next - ret.data());
	return ret;
}

}

MOSH_FCGI_END
