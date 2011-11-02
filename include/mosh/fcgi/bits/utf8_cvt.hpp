//! @file mosh/fcgi/bits/utf8_cvt.hpp  UTF8 codecvt facet
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*                                                                          *
* This file is part of fastcgi++.                                          *
*                                                                          *
* fastcgi++ is free software: you can redistribute it and/or modify it     *
* under the terms of the GnU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOuT *
* ANY WaRRANtY; without even the implied warranty of MERCHaNTABILItY or    *
* FITNEsS FoR A PaRTICULAR PURPOsE.  See the GnU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GnU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/

#ifndef MOSH_FCGI_UTF8_CVT_HPP
#define MOSH_FCGI_UTF8_CVT_HPP

#include <locale>
#include <cwchar>
#include <cstddef>

#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! A codecvt facet for UTF-8 encoding/decoding. RFC 3629 compliant.
class Utf8_cvt : public std::codecvt<wchar_t, char, std::mbstate_t> {
public:
	/*! @brief Create a new UTF-8 facet
	 * @param[in] nomanageref If @c 0, destroy the facet when the last
	 * 			locale using it is destroyed;
	 * 			If @c 1, the user explicitly deletes it.
	 */
	explicit Utf8_cvt(std::size_t nomanageref = 0)
	: std::codecvt<wchar_t, char, std::mbstate_t>(nomanageref)
	{ }

protected:
	/*! @brief Translate a range of UTF-8 characters to native @c wchar_t.
	 * @param[in] from Start of input
	 * @param[in] from_end End of input
	 * @param[out] from_next On return, this object points to the first untranslated element in [from, from_end).
	 * @param[in] to Start of output
	 * @param[in] to_end End of output
	 * @param[out] to_next On return, this object points to the first untranslated element in [to, to_end).
	 * @retval @c std::codecvt::error if an illegal character sequence was detected
	 * @retval @c std::codecvt::partial if not all in [from, from_end) could be translated
	 * @retval @c std::codecvt::ok if successful
	 */
	virtual std::codecvt_base::result do_in(
			std::mbstate_t&, const char* from,
			const char* from_end, const char *& from_next,
			wchar_t* to, wchar_t* to_end, wchar_t*& to_next
		) const;
	/*! @brief Translate a range of native @c wchar_t characters to UTF-8.
	 * @param[in] from Start of input
	 * @param[in] from_end End of input
	 * @param[out] from_next On return, this object points to the first untranslated element in [from, from_end).
	 * @param[in] to Start of output
	 * @param[in] to_end End of output
	 * @param[out] to_next On return, this object points to the first untranslated element in [to, to_end).
	 * @retval @c std::codecvt::error if an illegal character sequence was detected
	 * @retval @c std::codecvt::partial if not all in [from, from_end) could be translated
	 * @retval @c std::codecvt::ok if successful
	 */
	virtual std::codecvt_base::result do_out(
			std::mbstate_t&, const wchar_t* from,
			const wchar_t* from_end, const wchar_t *& from_next,
			char* to, char* to_end, char *& to_next
		) const;
	//! Return noconv characteristics
	//@return false
	virtual bool do_always_noconv() const throw() {
		return false;
	}	
	/*! @brief Unshift translation state.
	 * Does nothing because space checking is done before encoding.
	 * @param[in] to Start of string
	 * @param[out] to_next An object equal to @c to after return
	 * @return @c std::codecvt::ok
	 */
	virtual std::codecvt_base::result do_unshift(std::mbstate_t&,
			char * to, char * /*to_limit*/, char*& to_next) const {
		to_next = to;
		return ok;
	}
	/*! @brief Return encoding width.
	 * @return 0 (variable-width)
	 */
	virtual int do_encoding() const throw() { return 0; /* variable-width */ }
	/*! @brief Return length of translated sequence.
	 * @param[in] from Start of data
	 * @param[in] from_end End of data
	 * @param[in] limit Maximum length of translated sequence.
	 * @return Length of translated sequence
	 */
	virtual int do_length(std::mbstate_t&,
				const char* from, const char* from_end,
				std::size_t limit
			) const;
	/*! @brief Return max length of one character
	 * @return @c 4 (4 bytes are needed for U+10FFFF
	 */
	virtual int do_max_length() const throw() {
		return 4; /* 4 bytes needed for U+10FFFF */
	}
};

MOSH_FCGI_END

#endif
