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
#include <tuple>
#include <mosh/fcgi/html/sgml_doctype.hpp>
#include <mosh/fcgi/bits/t_string.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! HTML classes
namespace html {

//! HTML Doctype classes
namespace html_doctype {

//! HTML revision
namespace html_revision_atom {
	
//! Product family
enum class Family {
	//! HTML and variants thereof
	html,
	//! XHTML and variants thereof
	xhtml,
};

//! Per-family products
enum class Product {
	//! Default value
	none,
	//! XHTML Basic
	xhtml_basic,
	//! XHTML Mobile Profile
	xhtml_mp,
};

//! Per-product versions
enum class Version {
	//! HTML 4
	html_4,
	//! HTML 5
	html_5,
	//! XHTML 1.0
	xhtml_1_0,
	//! XHTML 1.1
	xhtml_1_1,
	//! XHTML 5
	xhtml_5,
	//! XHTML Basic 1.0
	xhtml_basic_1_0,
	//! XHTML Basic 1.1
	xhtml_basic_1_1,
	//! XHTML mobile profile 1.0
	xhtml_mp_1_0,
	//! XHTML mobile profile 1.1
	xhtml_mp_1_1,
	//! XHTML mobile profile 1.2
	xhtml_mp_1_2,
};

//! Variants
enum class Variant {
	//! Default value
	none,
	//! HTML4 / XHTML 1.0 Strict
	strict,
	//! HTML4 / XHTML 1.0 Transitional
	transitional,
	//! HTML4 / XHTML 1.0 Frameset
	frameset,
};

}

typedef std::tuple<html_revision_atom::Family, html_revision_atom::Product, html_revision_atom::Version, html_revision_atom::Variant> HTML_revision;

namespace html_revisions {

#define MOSH_FCGI_HTML_REVISION_FV(FAMILY, VERSION) \
	HTML_revision( \
		html_revision_atom::Family::FAMILY, \
		html_revision_atom::Product::none, \
		html_revision_atom::Version::VERSION, \
		html_revision_atom::Variant::none \
	)

#define MOSH_FCGI_HTML_REVISION_FPV(FAMILY, PRODUCT, VERSION) \
	HTML_revision( \
		html_revision_atom::Family::FAMILY, \
		html_revision_atom::Product::PRODUCT, \
		html_revision_atom::Version::VERSION, \
		html_revision_atom::Variant::none \
	)

#define MOSH_FCGI_HTML_REVISION_FVV(FAMILY, VERSION, VARIANT) \
	HTML_revision( \
		html_revision_atom::Family::FAMILY, \
		html_revision_atom::Product::none, \
		html_revision_atom::Version::VERSION, \
		html_revision_atom::Variant::VARIANT \
	)

#define MOSH_FCGI_HTML_REVISION_FPVV(FAMILY, PRODUCT, VERSION, VARIANT) \
	HTML_revision( \
		html_revision_atom::Family::FAMILY, \
		html_revision_atom::Product::PRODUCT, \
		html_revision_atom::Version::VERSION, \
		html_revision_atom::Variant::VARIANT \
	)

//! HTML 4.01 (Strict)
const HTML_revision html_4 = MOSH_FCGI_HTML_REVISION_FVV(html, html_4, strict);
//! HTML 4.01 Strict
const HTML_revision html_4_strict = MOSH_FCGI_HTML_REVISION_FVV(html,html_4, strict);
//! HTML 4.01 Frameset
const HTML_revision html_4_frameset = MOSH_FCGI_HTML_REVISION_FVV(html, html_4, frameset);
//! HTML 4.01 Transitional
const HTML_revision html_4_transitional = MOSH_FCGI_HTML_REVISION_FVV(html, html_4, transitional);
//! XHTML 1.0 (Strict)
const HTML_revision xhtml_10 = MOSH_FCGI_HTML_REVISION_FVV(xhtml, xhtml_1_0, strict);
//! XHTML 1.0 Strict
const HTML_revision xhtml_10_strict = MOSH_FCGI_HTML_REVISION_FVV(xhtml, xhtml_1_0, strict);
//! XHTML 1.0 Frameset
const HTML_revision xhtml_10_frameset = MOSH_FCGI_HTML_REVISION_FVV(xhtml, xhtml_1_0, frameset);
//! XHTML 1.0 Transitional
const HTML_revision xhtml_10_transitional = MOSH_FCGI_HTML_REVISION_FVV(xhtml, xhtml_1_0, transitional);
//! XHTML 1.1
const HTML_revision xhtml_11 = MOSH_FCGI_HTML_REVISION_FV(xhtml, xhtml_1_1);
//! XHTML Basic 1.0
const HTML_revision xhtml_basic_10 = MOSH_FCGI_HTML_REVISION_FPV(xhtml, xhtml_basic, xhtml_basic_1_0);
//! XHTML Basic 1.1
const HTML_revision xhtml_basic_11 = MOSH_FCGI_HTML_REVISION_FPV(xhtml, xhtml_basic, xhtml_basic_1_1);
//! XHTML Mobile Profile 1.0
const HTML_revision xhtml_mp_10 = MOSH_FCGI_HTML_REVISION_FPV(xhtml, xhtml_mp, xhtml_mp_1_0);
//! XHTML Mobile Profile 1.1
const HTML_revision xhtml_mp_11 = MOSH_FCGI_HTML_REVISION_FPV(xhtml, xhtml_mp, xhtml_mp_1_1);
//! XHTML Mobile Profile 1.2
const HTML_revision xhtml_mp_12 = MOSH_FCGI_HTML_REVISION_FPV(xhtml, xhtml_mp, xhtml_mp_1_2);
//! HTML 5
const HTML_revision html_5 = MOSH_FCGI_HTML_REVISION_FV(html, html_5);
//! XHTML 5
const HTML_revision xhtml_5 = MOSH_FCGI_HTML_REVISION_FV(xhtml, xhtml_5);

#undef MOSH_FCGI_HTML_REVISION_FV
#undef MOSH_FCGI_HTML_REVISION_FPV
#undef MOSH_FCGI_HTML_REVISION_FVV
#undef MOSH_FCGI_HTML_REVISION_FPVV
}

template <typename charT>
std::basic_string<charT> html_identifier(HTML_revision const& hr) {
	return wide_string<charT>(html_identifier<char>(hr));
}
template<> std::string html_identifier<char>(HTML_revision const& hr);

template <typename charT>
sgml_doctype::Doctype_declaration<charT> html_doctype(HTML_revision const& hr) {
	return sgml_doctype::Doctype_declaration<charT>(wide_string<charT>("html"), html_identifier<charT>(hr));
}

}
}

MOSH_FCGI_END

#endif
