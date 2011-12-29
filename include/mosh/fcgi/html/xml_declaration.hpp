//! @file  mosh/fcgi/html/xml_declaration.hpp XML Declaration
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

#ifndef MOSH_FCGI_HTML_XML_DECLARATION_HPP
#define MOSH_FCGI_HTML_XML_DECLARATION_HPP

#include <string>
#include <sstream>
#include <stdexcept>
#include <map>
#include <utility>
#include <mosh/fcgi/bits/t_string.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! HTML classes
namespace html {

//!  XML Declaration
template <typename charT>
class XML_declaration {
public:
	//! Typedef for strings
	typedef typename std::basic_string<charT> string;
	//! Typedef for attributes
	typedef typename std::pair<string, string> attribute;
	//! Typedef for attribute lists
	typedef typename std::map<string, string> attr_list;
private:
	typedef XML_declaration<charT> this_type;
public:
	//! Default constructor
	XML_declaration()
	{
		attributes.insert(std::make_pair("version", "1.0"));
	}
	//! Copy constructor
	XML_declaration(const XML_declaration& x)
	: attributes(x.attributes)
	{ }
	//! Move constructor
	XML_declaration(XML_declaration&& x)
	: attributes(std::move(x.attributes))
	{ }
	//! Destructor
	virtual ~XML_declaration() { }

	//! Assignment operator
	this_type& operator = (const this_type& x) {
		if (this != &x) {
			attributes = x.attributes;
		}
		return *this;
	}

	//! Assignment operator
	this_type operator = (this_type&& x) {
		if (this != &x) {
			attributes = std::move(x.attributes);
		}
		return *this;
	}
	
	/*! @name Clone and call
	 * These overloads create new declarations which behave as if attributes and values 
	 * were appended to existing declarations.
	 */
	//@{
	/*! @brief Create a copy of @c *this.
	 */
	this_type operator () () const {
		return this_type(*this);
	}

	/*! @brief Create a copy of @c *this with a given attribute.
	 *  @param[in] _a attribute
	 */
	this_type operator () (const attribute& _a) const {
		this_type e(*this);
		e += _a;
		return e;
	}

	/*! @brief Create a copy of @c *this with given attribute(s).
	 *  @param[in] _a-{} list of attributes
	 */
	this_type operator () (std::initializer_list<attribute> _a) const {
		this_type e(*this);
		e += _a;
		return e;
	}
	//@}
	/*! @name Appenders
	 */
	//@{
	/*! @brief Add an attribute.
	 *  @param[in] _a attribute
	 */
	this_type& operator += (const attribute& _a) {
		attributes.insert(_a);
		return *this;
	}

	/*! @brief Add attribute(s).
	 * @param[in] _a {}-list of attributes
	 */
	this_type& operator += (std::initializer_list<attribute> _a) {
		for (const auto& at : _a) {
			attributes.insert(at);
		}
		return *this;
	}
	//@}

	/*! @brief String cast operator
	 *  Renders the declaration, with attributes and pre-rendered data.
	 *  @warning No escaping is done.
	 */
	operator string() const {
		std::basic_stringstream<charT> ss;
		ss << wide_string<charT>("<?xml");
		for (const auto& at : attributes) {
			ss << wide_char<charT>(' ');
			ss << at.first;
			ss << wide_string<charT>("=\"");
			ss << at.second;
			ss << wide_char<charT>('"');
		}
		ss << wide_string<charT>(" ?>");
		return ss.str();
	}

	//! String cast operator
	operator const charT* () const {
		return this->operator string().c_str();
	}

private:
	attr_list attributes;
};

/*! @name Concatenators
 * These overloads create new declarations which behave as if 
 * attributes and values were appended to existing declarations.
 */
//@{
/*! @brief Concatenate an attribute.
 *  @param[in] _e declaration
 *  @param[in] _a attribute
 */
template <typename charT>
XML_declaration<charT> operator + (const XML_declaration<charT>& _e,
					const typename XML_declaration<charT>::attribute& _a)
{
	XML_declaration<charT> e(_e);
	e += _a;
	return e;
}

/*! @brief Concatenate an attribute.
 *  @param[in] _e declaration
 *  @param[in] _a attribute
 */
template <typename charT>
XML_declaration<charT> operator + (XML_declaration<charT>&& _e,
					const typename XML_declaration<charT>::attribute& _a)
{
	XML_declaration<charT> e(_e);
	e += _a;
	return std::move(e);
}

/*! @brief Concatenate attribute(s).
 *  @param[in] _e declaration
 *  @param[in] _a {}-list of attributes
 */
template <typename charT>
XML_declaration<charT> operator + (const XML_declaration<charT>& _e,
					std::initializer_list<typename XML_declaration<charT>::attribute> _a)
{
	XML_declaration<charT> e(_e);
	e += _a;
	return e;
}

/*! @brief Concatenate attribute(s).
 *  @param[in] _e declaration
 *  @param[in] _a {}-list of attributes
 */
template <typename charT>
XML_declaration<charT> operator + (XML_declaration<charT>&& _e,
					std::initializer_list<typename XML_declaration<charT>::attribute> _a)
{
	XML_declaration<charT> e(_e);
	e += _a;
	return std::move(e);
}

}

MOSH_FCGI_END

#endif
