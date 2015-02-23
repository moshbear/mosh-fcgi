//! @file html/html_doctype.cpp HTML DTD Declaration
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

#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <mosh/fcgi/html/sgml_doctype.hpp>
#include <mosh/fcgi/html/html_doctype.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {

//! Make string from literal
template <typename T, size_t N>
constexpr std::basic_string<T> S(const T (&s)[N]) {
        return std::basic_string<T>(s, N);
}

using namespace MOSH_FCGI::html::html_doctype;

using namespace html_revision_atom;

const std::map<Family, std::map<Product, std::map<Version, std::map<Variant, std::pair<std::string, std::string>>>>>
html_string = {
	{ Family::html, {
		{ Product::none, {
			{ Version::html_4, {
				{ Variant::strict, {
					S("-//W3C//DTD HTML 4.01//EN"),
					S("http://www.w3.org/TR/html4/strict.dtd")
				}},
				{ Variant::transitional, {
					S("-//W3C//DTD HTML 4.01 Transitional//EN"),
					S("http://www.w3.org/TR/html4/loose.dtd")
				}},
				{ Variant::frameset, {
					S("-//W3C//DTD HTML 4.01 Frameset//EN"),
					S("http://www.w3.org/TR/html4/frameset.dtd")
				}}
			}},
			{ Version::html_5, {
				{ Variant::none, {
					S(""),
					S("")
				}}
			}}
		}}
	}},
	{ Family::xhtml, {
		{ Product::none, {
			{ Version::xhtml_1_0, {
				{ Variant::strict, {
					S("-//W3C//DTD XHTML 1.0 Strict//EN"),
					S("http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd")
				}},
				{ Variant::transitional, {
					S("-//W3C//DTD XHTML 1.0 Transitional//EN"),
					S("http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd")
				}},
				{  Variant::frameset, {
					S("-//W3C//DTD XHTML 1.0 Frameset//EN"),
					S("http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd")
				}}
			}},
			{ Version::xhtml_1_1, {
				{ Variant::none, {
					S("-//W3C//DTD XHTML 1.1//EN"),
					S("http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd")
				}}
			}},
			{ Version::xhtml_5, {
				{ Variant::none, {
					S(""),
					S("")
				}}
			}}
		}},
		{ Product::xhtml_basic, {
			{ Version::xhtml_basic_1_0, {
				{ Variant::none, {
					S("-//W3C//DTD XHTML Basic 1.0//EN"),
					S("http://www.w3.org/TR/xhtml-basic/xhtml-basic10.dtd")
				}},
			}},
			{ Version::xhtml_basic_1_1, {
				{ Variant::none, {
					S("-//W3C//DTD XHTML Basic 1.1//EN"),
					S("http://www.w3.org/TR/xhtml-basic/xhtml-basic11.dtd")
				}},
			}}
		}},
		{ Product::xhtml_mp, {
			{ Version::xhtml_mp_1_0, {
				{ Variant::none, {
					S("-//WAPFORUM//DTD XHTML Mobile 1.0//EN"),
					S("http://www.wapforum.org/DTD/xhtml-mobile10.dtd")
				}}
			}},
			{ Version::xhtml_mp_1_1, {
				{ Variant::none, {
					S("-//WAPFORUM//DTD XHTML Mobile 1.1//EN"),
					S("http://www.wapforum.org/DTD/xhtml-mobile11.dtd")
				}}
			}},
			{ Version::xhtml_mp_1_2, {
				{ Variant::none, {
					S("-//WAPFORUM//DTD XHTML Mobile 1.2//EN"),
					S("http://www.wapforum.org/DTD/xhtml-mobile12.dtd")
				}}
			}}
		}}
	}}
};

}

MOSH_FCGI_BEGIN

namespace html {

namespace html_doctype {

template<> std::string html_identifier<char>(HTML_revision const& hr) {
	using namespace sgml_doctype;

	try {
		auto str = html_string.at(std::get<0>(hr))
					.at(std::get<1>(hr))
					.at(std::get<2>(hr))
					.at(std::get<3>(hr));
		External_doctype_identifier<char> d(Declaration_identifier::_public, str.first, str.second);
		return d;
	} catch (std::out_of_range&) {
		throw std::invalid_argument("invalid html identifier");
	}
}

}

}

MOSH_FCGI_END
