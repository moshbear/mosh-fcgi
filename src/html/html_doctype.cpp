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

#include <stdexcept>
#include <string>
#include <utility>
#include <mosh/fcgi/html/sgml_doctype.hpp>
#include <mosh/fcgi/html/html_doctype.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {

std::pair<std::string, std::string> w3c_fpi_uri(unsigned hr) {
	using namespace MOSH_FCGI::html::html_doctype::html_revision;
	using std::make_pair;

	switch (hr) {
	case html_4_strict:
		return make_pair("-//W3C//DTD HTML 4.01//EN", "http://www.w3.org/TR/html4/strict.dtd");
	case html_4_frameset:
		return make_pair("-//W3C//DTD HTML 4.01 Frameset//EN", "http://www.w3.org/TR/html4/frameset.dtd");
	case html_4_transitional:
		return make_pair("-//W3C//DTD HTML 4.01 Transitional//EN", "http://www.w3.org/TR/html4/loose.dtd");
	case xhtml_10_strict:
		return make_pair("-//W3C//DTD XHTML 1.0 Strict//EN", "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd");
	case xhtml_10_frameset:
		return make_pair("-//W3C//DTD XHTML 1.0 Frameset//EN", "http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd");
	case xhtml_10_transitional:
		return make_pair("-//W3C//DTD XHTML 1.0 Transitional//EN", "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd");
	case xhtml_11:
		return make_pair("-//W3C//DTD XHTML 1.1//EN", "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd");
	case xhtml_basic_10:
		return make_pair("-//W3C//DTD XHTML Basic 1.0//EN", "http://www.w3.org/TR/xhtml-basic/xhtml-basic10.dtd");
	case xhtml_basic_11:
		return make_pair("-//W3C//DTD XHTML Basic 1.1//EN", "http://www.w3.org/TR/xhtml-basic/xhtml-basic11.dtd");
	case xhtml_mp_10:
		return make_pair("-//WAPFORUM//DTD XHTML Mobile 1.0//EN", "http://www.wapforum.org/DTD/xhtml-mobile10.dtd");
	case xhtml_mp_11:
		return make_pair("-//WAPFORUM//DTD XHTML Mobile 1.1//EN", "http://www.wapforum.org/DTD/xhtml-mobile11.dtd");
	case xhtml_mp_12:
		return make_pair("-//WAPFORUM//DTD XHTML Mobile 1.2//EN", "http://www.wapforum.org/DTD/xhtml-mobile12.dtd");
	case html_5:
		return make_pair("", ""); // HTML 5 is DTD-free
	default:;
	}
	throw std::invalid_argument("doctype not found");
}

}

MOSH_FCGI_BEGIN

namespace html {
namespace html_doctype {

template<> std::string html_identifier<char>(unsigned hr) {
	using namespace sgml_doctype;
	auto fpi_uri = w3c_fpi_uri(hr);
	External_doctype_identifier<char> d(Declaration_identifier::_public, fpi_uri.first, fpi_uri.second);
	return d;
}

}
}

MOSH_FCGI_END
