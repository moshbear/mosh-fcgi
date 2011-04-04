//! \file unicode.hpp - UTF-8 encode/decode
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
#ifndef FASTCGIPP_UNICODE_HPP
#define FASTCGIPP_UNICODE_HPP

#include <string>

namespace Fastcgipp_m0sh {

namespace Unicode {

/*!
 * \brief Convert UTF-8 byte strings to the internal string type.
 *
 * If the template parameter is \c char, then nothing is done.
 * If the template parameter is \c wchar_t, then the UTF-8 input facet is used
 * to decode the string into Unicode code points.
 * \tparam charT character type; see above for details
 * \param s input string
 * \return Unicode representation of input
 */
template <class charT>
std::basic_string<charT> utfIn(const std::string& s);
template<> std::wstring utfIn<wchar_t>(const std::string&);
template<> inline std::string utfIn<char>(const std::string& s) { return s; }
/*!
 * \brief Convert the internal string type to UTF-8.
 *
 * If the deduced template parameter is \c wchar_t, then the UTF-8 output facet is used
 * to encode the string into UTF-8.
 * If the deduced template parameter is \c char, then nothing is done.
 * \tparam charT (deduced from input string type)
 * \param s input string
 * \return UTF-8 representation of input
 */
template <class charT>
std::string utfOut(const std::basic_string<charT>& s);
template<> std::string utfOut<wchar_t>(const std::wstring&);
template<> inline std::string utfOut<char>(const std::string& s) { return s; }

}

}

#endif
