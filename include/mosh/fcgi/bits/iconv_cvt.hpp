//! @file mosh/fcgi/bits/iconv_cvt.hpp Iconv codecvt facet
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

#ifndef MOSH_FCGI_ICONV_CVT_HPP
#define MOSH_FCGI_ICONV_CVT_HPP

#include <locale>
#include <cwchar>
#include <stdexcept>
#include <cstddef>

#include <mosh/fcgi/bits/iconv.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! A codecvt facet wrapper for Iconv.
template <typename in_type, typename ex_type>
class Iconv_cvt : public std::codecvt<in_type, ex_type, Iconv::IC_state*> {
public:
	/*! @brief Create a new Iconv facet
	 * @param[in] nomanageref If @c 0, destroy the facet when the last
	 * 			locale using it is destroyed;
	 * 			If @c 1, the user explicitly deletes it.
	 */
	explicit Iconv_cvt(std::size_t nomanageref = 0)
	: std::codecvt<in_type, ex_type, Iconv::IC_state*>(nomanageref)
	{ }

protected:
	/*! @brief Translate ex_types, according to converter state.
	 * @param[in] ic Iconv state
	 * @param[in] from Start of input
	 * @param[in] from_end End of input
	 * @param[out] from_next On return, this object points to the first untranslated element in [from, from_end).
	 * @param[in] to Start of output
	 * @param[in] to_end End of output
	 * @param[out] to_next On return, this object points to the first untranslated element in [to, to_end).
	 * @retval @c std::codecvt::error if an illegal character sequence was detected
	 * @retval @c std::codecvt::partial if not all in [from, from_end) could be translated
	 * @retval @c std::codecvt::ok if successful
	 * @retval @c std::codecvt::noconv if no conversion was performed
	 */
	virtual std::codecvt_base::result do_in(
			Iconv::IC_state*& ic, const ex_type* from,
			const ex_type* from_end, const ex_type *& from_next,
			in_type* to, in_type* to_end, in_type*& to_next
		) const;
	/*! @brief Translate a range of native @c in_type characters to UTF-8.
	 * @param[in] ic Iconv state
	 * @param[in] from Start of input
	 * @param[in] from_end End of input
	 * @param[out] from_next On return, this object points to the first untranslated element in [from, from_end).
	 * @param[in] to Start of output
	 * @param[in] to_end End of output
	 * @param[out] to_next On return, this object points to the first untranslated element in [to, to_end).
	 * @retval @c std::codecvt::error if an illegal character sequence was detected
	 * @retval @c std::codecvt::partial if not all in [from, from_end) could be translated
	 * @retval @c std::codecvt::ok if successful
	 * @retval @c std::codecvt::noconv if no conversion was performed
	 */
	virtual std::codecvt_base::result do_out(
			Iconv::IC_state*& ic, const in_type* from,
			const in_type* from_end, const in_type *& from_next,
			ex_type* to, ex_type* to_end, ex_type *& to_next
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
	virtual std::codecvt_base::result do_unshift(Iconv::IC_state*&,
			ex_type* to, ex_type* /*to_limit*/, ex_type*& to_next) const
	{
		to_next = to;
		return std::codecvt_base::ok;
	}
	/*! @brief Return encoding width.
	 * @return 0 (variable-width)
	 */
	virtual int do_encoding() const throw() { return 0; /* variable-width */ }
	/*! @brief Return length of translated sequence.
	 * @throw std::logic_error
	 */
	virtual int do_length(Iconv::IC_state*&, const ex_type*, const ex_type*,
				std::size_t) const {
		throw std::logic_error("Impossible to determine string length");
	}
	/*! @brief Return max length of one character
	 * @return @c -1 (state-dependent)
	 */
	virtual int do_max_length() const throw() {
		return -1;
	}
};

MOSH_FCGI_END

#endif

