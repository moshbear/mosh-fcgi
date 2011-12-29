//! @file  mosh/fcgi/html/html_doctype.hpp HTML DTD Declaration
/* 
 *  Copyright (C) 2011 m0shbear
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 */

#ifndef MOSH_FCGI_HTML_HTML_DOCTYPE_HPP
#define MOSH_FCGI_HTML_HTML_DOCTYPE_HPP

#include <string>
#include <cstdint>
#include <mosh/fcgi/html/sgml_doctype.hpp>
#include <mosh/fcgi/bits/t_string.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! HTML classes
namespace html {

//! HTML Doctype classes
namespace html_doctype {

//! HTML revision
namespace html_revision {
	
/*! @brief Make a bitwise HTML revision
 *  @param _family Product family
 *  @param _product Product
 *  @param _version Product version
 *  @param _variant Product variant
 *
 *  @see Family
 *  @see product
 *  @see version
 *  @see variant
 */
constexpr uint32_t make_revision(uint8_t _family, uint8_t _product, uint8_t _version, uint8_t _variant) {
	return (_family << 24) | (_product << 16) | (_version << 8) | (_variant);
}
/*! @brief Make a bitwise HTML revision
 *  @param _family Product family
 *  @param _product Product
 *  @param _version Product version
 *
 *  @see Family
 *  @see product
 *  @see version
 */
template <typename T1, typename T2, typename T3>
constexpr uint32_t make_revision(T1 _family, T2 _product, T3 _version) {
	return make_revision(
		static_cast<uint8_t>(_family),
		static_cast<uint8_t>(_product),
		static_cast<uint8_t>(_version),
		static_cast<uint8_t>(0)
	);
}
/*! @brief Make a bitwise HTML revision
 *  @param _family Product family
 *  @param _product Product
 *  @param _version Product version
 *  @param _variant Product variant
 *
 *  @see Family
 *  @see product
 *  @see version
 *  @see variant
 */
template <typename T1, typename T2, typename T3, typename T4>
constexpr uint32_t make_revision(T1 _family, T2 _product, T3 _version, T4 _variant) {
	return make_revision(
		static_cast<uint8_t>(_family),
		static_cast<uint8_t>(_product),
		static_cast<uint8_t>(_version),
		static_cast<uint8_t>(_variant)
	);
}

//! Get the encoded family code in hr
constexpr uint8_t get_family(uint32_t hr) {
	return static_cast<uint8_t>((hr & 0xFF000000) >> 24);
}

//! Get the encoded product code in hr
constexpr uint8_t get_product(uint32_t hr) {
	return static_cast<uint8_t>((hr & 0x00FF0000) >> 16);
}

//! Get the encoded version code in hr
constexpr uint8_t get_version(uint32_t hr) {
	return static_cast<uint8_t>((hr & 0x0000FF00) >> 8);
}

//! Get the encoded variant code in hr
constexpr uint8_t get_variant(uint32_t hr) {
	return static_cast<uint8_t>((hr & 0x000000FF));
}

//! Product family
enum class Family : uint8_t {
	//! HTML and variants thereof
	html = 1,
	//! XHTML and variants thereof
	xhtml = 2,
};

//! Per-family products
namespace product {
//! XHTML product family
enum class XHTML : uint8_t {
	//! XHTML Basic
	basic = 1,
	//! XHTML Mobile Profile
	mp = 2,
};
}

//! Per-product versions
namespace version {
//! HTML version family
enum class HTML : uint8_t {
	//! HTML 4
	_4 = 1,
	//! HTML 5
	_5 = 2,
};	
//! XHTML version family
enum class XHTML : uint8_t {
	//! XHTML 1.0
	_1_0 = 1,
	//! XHTML 1.1
	_1_1 = 2,
	//! XHTML 5
	_5 = 3,
};
//! XHTML Basic version family
enum class XHTML_basic : uint8_t {
	//! XHTML Basic 1.0
	_1_0 = 1,
	//! XHTML Basic 1.1
	_1_1 = 2,
};
//! XHTML Mobile Profile version family
enum class XHTML_mp : uint8_t {
	//! XHTML mobile profile 1.0
	_1_0 = 1,
	//! XHTML mobile profile 1.1
	_1_1 = 2,
	//! XHTML mobile profile 1.2
	_1_2 = 3,
};
}

//! Variants
namespace variant {
//! HTML Variant; only useful for HTML 4 and XHTML 1.0
enum class HTML4 : uint8_t {
	strict = 1,
	transitional = 2,
	frameset = 3,
};
}

//! HTML 4.01 (Strict)
const uint32_t html_4 = make_revision(Family::html, 0, version::HTML::_4, variant::HTML4::strict);
//! HTML 4.01 Strict
const uint32_t html_4_strict = make_revision(Family::html, 0, version::HTML::_4, variant::HTML4::strict);
//! HTML 4.01 Frameset
const uint32_t html_4_frameset = make_revision(Family::html, 0, version::HTML::_4, variant::HTML4::frameset);
//! HTML 4.01 Transitional
const uint32_t html_4_transitional = make_revision(Family::html, 0, version::HTML::_4, variant::HTML4::transitional);
//! XHTML 1.0 (Strict)
const uint32_t xhtml_10 = make_revision(Family::xhtml, 0, version::XHTML::_1_0, variant::HTML4::strict);
//! XHTML 1.0 Strict
const uint32_t xhtml_10_strict = make_revision(Family::xhtml, 0, version::XHTML::_1_0, variant::HTML4::strict);
//! XHTML 1.0 Frameset
const uint32_t xhtml_10_frameset = make_revision(Family::xhtml, 0, version::XHTML::_1_0, variant::HTML4::frameset);
//! XHTML 1.0 Transitional
const uint32_t xhtml_10_transitional = make_revision(Family::xhtml, 0, version::XHTML::_1_0, variant::HTML4::transitional);
//! XHTML 1.1
const uint32_t xhtml_11 = make_revision(Family::xhtml, 0, version::XHTML::_1_1);
//! XHTML Basic 1.0
const uint32_t xhtml_basic_10 = make_revision(Family::xhtml, product::XHTML::basic, version::XHTML_basic::_1_0);
//! XHTML Basic 1.1
const uint32_t xhtml_basic_11 = make_revision(Family::xhtml, product::XHTML::basic, version::XHTML_basic::_1_1);
//! XHTML Mobile Profile 1.0
const uint32_t xhtml_mp_10 = make_revision(Family::xhtml, product::XHTML::mp, version::XHTML_mp::_1_0);
//! XHTML Mobile Profile 1.1
const uint32_t xhtml_mp_11 = make_revision(Family::xhtml, product::XHTML::mp, version::XHTML_mp::_1_1);
//! XHTML Mobile Profile 1.2
const uint32_t xhtml_mp_12 = make_revision(Family::xhtml, product::XHTML::mp, version::XHTML_mp::_1_2);
//! HTML 5
const uint32_t html_5 = make_revision(Family::html, 0, version::HTML::_4);

}

template <typename charT>
std::basic_string<charT> html_identifier(unsigned hr) {
	return wide_string<charT>(html_identifier<char>(hr));
}
template<> std::string html_identifier<char>(unsigned hr);

template <typename charT>
sgml_doctype::Doctype_declaration<charT> html_doctype(unsigned hr) {
	return sgml_doctype::Doctype_declaration<charT>(wide_string<charT>("html"), html_identifier<charT>(hr));
}

}
}

MOSH_FCGI_END

#endif
