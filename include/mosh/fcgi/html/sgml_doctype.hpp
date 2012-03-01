//! @file  mosh/fcgi/html/sgml_doctype.hpp SGML DTD Declaration
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

#ifndef MOSH_FCGI_HTML_SGML_DOCTYPE_HPP
#define MOSH_FCGI_HTML_SGML_DOCTYPE_HPP

#include <ostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! HTML classes
namespace html {

//! SGML Doctype classes
namespace sgml_doctype {

//! Doctype declaration identifier
enum class Declaration_identifier {
	//! Identifies a document type that is used exclusively in one application
	_system,
	//! Identifies a document type that may span more than one application
	_public,
};

//! External doctype identifier
template <class charT>
class External_doctype_identifier {
public:
	typedef std::basic_string<charT> string;
	/*! @brief Create an external doctype identifier
	 *  @param d_type Identifier type
	 *  @param qs1 first quoted string
	 *  @param qs2 second quoted string
	 *  
	 *  The value of d_type governs the behavior of @c qs1 and @c qs2. If @c Declaration_identifier::_system, then
	 *  @c qs1 is the URI
	 *  If @c Declaration_identifier::_public, then @c qs1 is the FPI and @c qs2 is the URI.
	 *  @see Declaration_identifier
	 */
	External_doctype_identifier(Declaration_identifier d_type, const string& qs1, const string& qs2 = string())
	: type(d_type), quoted1(qs1), quoted2(qs2)
	{
		switch (type) {
		case Declaration_identifier::_public:
			if (qs2.empty()) {
				throw std::invalid_argument("external identifiers is PUBLIC and qs2 is empty");
			}
			break;
		case Declaration_identifier::_system: break;
		}

	}

	virtual ~External_doctype_identifier() { }

	//! Convert to string
	operator string() const {
		switch (type) {
		case Declaration_identifier::_system:
			return ([&quoted1] () {
					std::basic_stringstream<charT> ss;
					ss << "SYSTEM \"";
					ss << quoted1;
					ss << '"';
					return ss.str();
				})();
		case Declaration_identifier::_public:
			return ([&quoted1, &quoted2] () {
					std::basic_stringstream<charT> ss;
					ss << "PUBLIC \"";
					ss << quoted1;
					ss << "\" \"";
					ss << quoted2;
					ss << '"';
					return ss.str();
				})();
		}
		return string();
	}
private:
	//! DTD type
	Declaration_identifier type;
	//! FPI if public, URI if system
	string quoted1;
	//! URI if public
	string quoted2;
};

//! Print to ostream
template <typename charT>
std::basic_ostream<charT>& operator << (std::basic_ostream<charT>& os, External_doctype_identifier<charT> const& e) {
	os << static_cast<std::basic_string<charT>>(e);
	return os;
}
		
//! Doctype declaration
template <class charT>
class Doctype_declaration {
	typedef Doctype_declaration<charT> this_type;
public:
	typedef std::basic_string<charT> string;
	
	/*! @brief Create a new document type declaration
	 *  @param d_name Declaration name
	 *  @param d_ext External identifier
	 *  @param d_int Internal DTD (well-formed SGML DTD is assumed)
	 */
	Doctype_declaration(const string& d_name, const string& d_ext = string(), const string& d_int = string())
	: name(d_name), external(d_ext), internal(d_int)
	{ }

	//! Copy constructor
	Doctype_declaration(const this_type& d)
	: name(d.name), external(d.external), internal(d.internal)
	{ }
	//! Move constructor
	Doctype_declaration(this_type&& d)
	: name(std::move(d.name)), external(std::move(d.external)), internal(std::move(d.internal))
	{ }

	//! Destructor
	virtual ~Doctype_declaration() { }
	
	//! Copy assignment
	this_type& operator = (const this_type& d) {
		if (this != &d) {
			name = d.name;
			external = d.external;
			internal = d.internal;
		}
		return *this;
	}
	//! Move assignment
	this_type& operator = (this_type&& d) {
		if (this != &d) {
			name = std::move(d.name);
			external = std::move(d.external);
			internal = std::move(d.internal);
		}
		return *this;
	}

	//! Add more SGML DTD to the internal DTD
	this_type& operator += (const string& s) {
		this->internal += s;
		return *this;
	}

	//! Convert to string
	operator string () const {
		std::basic_stringstream<charT> ss;
		ss << name;
		ss << ' ';
		ss << external;
		if (!internal.empty()) {
			ss << "[\r\n";
			ss << internal;
			ss << "\r\n] ";
		}
		ss << " >\r\n";
		return ss.str();
	}

private:
	//! Root element name
	string name;
	//! External identifier
	string external;
	//! Internal DTD
	string internal;
};

template <typename charT>
Doctype_declaration<charT> operator + (Doctype_declaration<charT> const& _d, const std::basic_string<charT>& s) {
	Doctype_declaration<charT> d(_d);
	d += s;
	return d;
}

template <typename charT>
Doctype_declaration<charT> operator + (Doctype_declaration<charT> && _d, const std::basic_string<charT>& s) {
	Doctype_declaration<charT> d(std::move(_d));
	d += s;
	return std::move(d);
}

//! Print to ostream
template <typename charT>
std::basic_ostream<charT>& operator << (std::basic_ostream<charT>& os, Doctype_declaration<charT> const& d) {
	os << static_cast<std::basic_string<charT>>(d);
	return os;
}

}
}

MOSH_FCGI_END

#endif
