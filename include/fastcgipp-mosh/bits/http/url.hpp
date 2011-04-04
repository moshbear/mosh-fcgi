//! \file http/url.hpp - url decoding stuff
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


#ifndef FASTCGIPP_HTTP_URL_HPP
#define FASTCGIPP_HTTP_URL_HPP

#include <string>
#include <algorithm>
#include <cctype>
#include <fastcgipp-mosh/unicode.hpp>

namespace Fastcgipp_m0sh
{


namespace Http
{

namespace Url {

	char hexUnescape(char first, char second);

	template <class charT>
	class decode {
	public:
		/*! @brief Convert encoded characters in form data to normal ASCII/Unicode.
		 * @param[in] _Begin start of data
		 * @param[in] _End end of data
		 * @param[out] res store how many characters are missing
		 * @return the converted string
		 */
		template <class InputIterator>
		static std::basic_string<charT> _(InputIterator _Begin, InputIterator _End, unsigned& res);
		
		/*! @brief Convert encoded characters in form data to normal ASCII/Unicode.
		 * @param[in] src input string
		 * @param[out] res store how many characters are missing
		 * @return the converted string
		 */
		static std::basic_string<charT> _(const std::string& src, unsigned& res);

		/*! @brief Convert encoded characters in form data to normal ASCII/Unicode.
		 * @param[in] str start of data
		 * @param[in] len length of data
		 * @param[out] res store how many characters are missing
		 * @return the converted string
		 */
		static std::basic_string<charT> _(const char* str, size_t len, unsigned& res);
	private:
		static unsigned dummy;
	};
	
	template<> class decode<char> {
	public:
		template <class InputIterator>
		static std::string _(InputIterator _Begin, InputIterator _End, unsigned& res = dummy) {
			std::string result;
			while (_Begin != _End) {
				switch (*_Begin) {
				case '+':
					result += ' ';
					break;
				case '%':
					// don't assume well-formed input
					if (std::distance(_Begin, _End) > 2 && std::isxdigit(*(_Begin+1)) && std::isxdigit(*(_Begin+2))) {
						char ch = *++_Begin;
						result += hexUnescape(ch, *++_Begin);
					} else {
						if (std::distance(_Begin, _End) <= 2) {
							res = 3 - std::distance(_Begin, _End);
							return result;
						} else {
						// just pass % through untouched
							result += '%';
						}
					}
					break;
				default:	
					result += *_Begin;
					break;
				}
				_Begin++;
			}
			return result;
		}

		inline static std::string _(const std::string& src, unsigned& res = dummy) {
			return _(src.begin(), src.end(), res); 
		}

		inline static std::string _(const char* str, size_t len, unsigned& res = dummy) {
			return _(str, str + len, res);
		}
	private:
		static unsigned dummy;
	};

	template<> class decode<wchar_t> {
	public:
		template <class InputIterator>
		inline static std::wstring _(InputIterator _Begin, InputIterator _End, unsigned& res = dummy) {
			return Unicode::utfIn<wchar_t>(decode<char>::_(_Begin, _End, res));
		}
		
		inline static std::wstring _(const std::string& src, unsigned& res = dummy) {
			return Unicode::utfIn<wchar_t>(decode<char>::_(src, res));
		}

		inline static std::wstring _(const char* str, size_t len, unsigned& res = dummy) {
			return Unicode::utfIn<wchar_t>(decode<char>::_(str, len, res));
		}
	private:
		static unsigned dummy;
	};
	
}

}
}

#endif


