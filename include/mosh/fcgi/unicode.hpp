//! @file unicode.hpp - UTF-8 encode/decode
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*                                                                          *
* This file is part of fastcgi++.                                          *
*                                                                          *
* fastcgi++ is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/
#ifndef MOSH_FCGI_UNICODE_HPP
#define MOSH_FCGI_UNICODE_HPP

#include <string>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace unicode {

/*!
 * @brief Convert a UTF-8 string to the internal string type.
 * @param[in] begin Start of input
 * @param[in] end End of input
 * @param[out] next On return, this object points to the first untranslated element in [begin, end)
 * @tparam char_type output string type
 * @return Unicode representation of input
 */
template <class char_type>
std::basic_string<char_type> in(const char* begin, const char* end, const char*& next);

template<> inline std::string in<char>(const char* begin, const char* end, const char*&) {
	return std::string(begin, end);
}
template<> std::wstring in<wchar_t>(const char* begin, const char* end, const char*& next);

/*!
 * @brief Convert UTF-8 byte strings to the internal string type.
 *
 * If the template parameter is @c char, then nothing is done.
 * If the template parameter is @c wchar_t, then the UTF-8 input facet is used
 * to decode the string into Unicode code points.
 * @tparam char_type character type; see above for details
 * @param s input string
 * @return Unicode representation of input
 */
template <class char_type>
std::basic_string<char_type> in(const std::string& s);

template<> std::string in<char>(const std::string& s) {
	return s;
}
template<> std::wstring in<wchar_t>(const std::string& s) {
	const char* dummy;
	return in<wchar_t>(&(*s.begin()), &(*s.end()), dummy);
}

/*!
 * @brief Convert the internal string type to UTF-8.
 *
 * If the deduced template parameter is @c wchar_t, then the UTF-8 output facet is used
 * to encode the string into UTF-8.
 * If the deduced template parameter is @c char, then nothing is done.
 * @param[in] begin Start of input
 * @param[in] end End of input
 * @param[out] next On return, this object points to the first untranslated element in [begin, end)
 * @tparam char_type input string type (deduced from input string type)
 * @return UTF-8 representation of input
 */
template <class char_type>
std::string out(const char_type* begin, const char_type* end, const char_type*& next);

template<> inline std::string out(const char* begin, const char* end, const char*&) {
	return std::string(begin, end);
}
template<> std::string out(const wchar_t* begin, const wchar_t* end, const wchar_t*& next);

/*!
 * @brief Convert the internal string type to UTF-8.
 *
 * If the deduced template parameter is @c wchar_t, then the UTF-8 output facet is used
 * to encode the string into UTF-8.
 * If the deduced template parameter is @c char, then nothing is done.
 * @tparam char_type (deduced from input string type)
 * @param s input string
 * @return UTF-8 representation of input
 */
template <class char_type>
std::string out(const std::basic_string<char_type>& s);

template<> std::string out<wchar_t>(const std::wstring& ws) {
	const wchar_t* dummy;
	return out(&(*ws.begin()), &(*ws.end()), dummy);
}
template<> inline std::string out<char>(const std::string& s) { return s; }

}

MOSH_FCGI_END

#endif
