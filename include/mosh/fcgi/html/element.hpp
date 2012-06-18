//! @file  mosh/fcgi/html/element.hpp Class dealing with HTML elements
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

#ifndef MOSH_FCGI_HTML_ELEMENT_HPP
#define MOSH_FCGI_HTML_ELEMENT_HPP

#include <string>
#include <sstream>
#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <stdexcept>
#include <vector>
#include <ostream>
#include <type_traits>
#include <boost/lexical_cast.hpp>
#include <mosh/fcgi/html/html_doctype.hpp>
#include <mosh/fcgi/bits/popcount.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! HTML classes
namespace html {

//! HTML element classes
namespace element {

//! Enumeration for encoding type
namespace Type {
	
	/* Guide to constants:
	 * Each value must be a power of 2
	 */

	//! Encodes an unary &lt;foo /&gt; element; resulting tag will have attributes but no data
	const uint8_t unary = 1 << 0;
	//! Encodes a binary &lt;foo&gt;&lt;/foo&gt; element; resulting tag will have both attributes and data
	const uint8_t binary = 1 << 1;
	//! Encodes a DOCTYPE directive &lt;!FOO &gt;
	const uint8_t dtd = 1 << 2;
	//! Encodes a comment &lt;!-- foo --&gt;
	const uint8_t comment = 1 << 3;
	
	inline void _validate(uint8_t t) {
		if (t < (1 << 0) || t > (1 << 3))
			goto _throw_;
		if (popcount(t) != 1)
			goto _throw_;
		return;
	_throw_:
		throw std::invalid_argument("MOSH_FCGI::html::element::Type");
	}		
}

//! Wrapper for boost::lexical_cast<string>
template <typename ct>
std::basic_string<ct> from_pcchar(const char* s) {
	return boost::lexical_cast<std::basic_string<ct>>(s);
}

//! An HTML element
template <typename charT>
class Element  {
public:
	//! Typedef for strings
	typedef typename std::basic_string<charT> string;
	//! Typedef for attributes
	typedef typename std::pair<string, string> attribute;
	//! Typedef for attribute lists
	typedef typename std::map<string, string> attr_list;
	//! Typedef for string lists
	typedef typename std::vector<string> string_list;
private:
	typedef Element<charT> this_type;
public:
	/*! @brief Create a new element with a given name and type
	 *
	 *  @param[in] name_ Element name
	 *  @param[in] type_ Element type
	 *  @sa Type
	 */
	Element(uint8_t type_, const string& name_)
	: type(type_), name(name_), attributes(), data()
	{
		Type::_validate(type_);
	}

	//! Copy constructor
	Element(const this_type& e)
	: type(e.type), name(e.name), attributes(e.attributes), data(e.data)
	{ }

	//! Move constructor
	Element(this_type&& e)
	: type(e.type), name(std::move(e.name)), attributes(std::move(e.attributes)),
	  data(std::move(e.data))
	{ }

	//! Destructor
	virtual ~Element() { }

	//! Assignment operator
	this_type& operator = (const this_type& e) {
		if (this != &e) {
			type = e.type;
			attributes = e.attributes;
			data = e.data;
			name = e.name;
		}
		return *this;
	}
	//! Assignment operator
	this_type operator = (this_type&& e) {
		if (this != &e) {
			type = e.type;
			attributes = std::move(e.attributes);
			data = std::move(e.data);
			name = std::move(e.name);
		}
		return *this;
	}

	/*! @name Clone and call
	 *
	 * These overloads create new elements which behave as if attributes and values 
	 * were appended to existing elements.
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
	 *  @param[in] _a {}-list of attributes
	 */
	this_type operator () (std::initializer_list<attribute> _a) const {
		this_type e(*this);
		e += _a;
		return e;
	}
	
	/*! @brief Create a copy of @c *this with given attribute(s).
	 *  @param[in] _a map of attribute
	 */
	this_type operator () (attr_list const& _a) const {
		this_type e(*this);
		e += _a;
		return e;
	}
	
	/*! @brief Create a copy of @c *this with a given value.
	 *  @param[in] _v value
	 */
	this_type operator () (const string& _v) const {
		this_type e(*this);
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c *this with given value(s).
	 *  @param[in] _v {}-list of values
	 */
	this_type operator () (std::initializer_list<string> _v) const {
		this_type e(*this);
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c *this with given value(s).
	 *  @param[in] _v vector of values
	 */
	this_type operator () (string_list const& _v) const {
		this_type e(*this);
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with a given attribute and value.
	 *  @param[in] _a attribute
	 *  @param[in] _v value
	 */
	this_type operator () (const attribute& _a, const string& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}
	
	/*! @brief Create a copy of @c this with a given attribute and value(s).
	 *  @param[in] _a attribute
	 *  @param[in] _v {}-list of values
	 */
	this_type operator () (const attribute& _a, std::initializer_list<string> _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with a given attribute and value(s).
	 *  @param[in] _a attribute
	 *  @param[in] _v list of values
	 */
	this_type operator () (const attribute& _a, string_list const& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with given attribute(s) and a value.
	 *  @param[in] _a {}-list of attributes
	 *  @param[in] _v value
	 */
	this_type operator () (std::initializer_list<attribute> _a, const string& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with given attribute(s) and value(s).
	 *  @param[in] _a {}-list of attributes
	 *  @param[in] _v {}-list of values
	 */
	this_type operator () (std::initializer_list<attribute> _a, std::initializer_list<string> _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}
	/*! @brief Create a copy of @c this with given attribute(s) and value(s).
	 *  @param[in] _a {}-list of attributes
	 *  @param[in] _v list of values
	 */
	this_type operator () (std::initializer_list<attribute> _a, string_list const& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}
	
	/*! @brief Create a copy of @c this with given attribute(s) and a value.
	 *  @param[in] _a map of attributes
	 *  @param[in] _v value
	 */
	this_type operator () (attr_list const& _a, const string& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with given attribute(s) and value(s).
	 *  @param[in] _a list of attributes
	 *  @param[in] _v {}-list of values
	 */
	this_type operator () (attr_list const& _a, std::initializer_list<string> _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}
	/*! @brief Create a copy of @c this with given attribute(s) and value(s).
	 *  @param[in] _a list of attributes
	 *  @param[in] _v list of values
	 */
	this_type operator () (attr_list const& _a, string_list const& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}
	//@}
	/*! @name Appenders
	 */
	//@{
	/*! @brief Add an attribute.
	 *  @param[in] _a attribute
	 */
	virtual this_type& operator += (const attribute& _a) {
		if (this->attribute_addition_hook(_a))
			attributes.insert(_a);
		return *this;
	}

	/*! @brief Add attribute(s).
	 * @param[in] _a {}-list of attributes
	 */
	this_type& operator += (std::initializer_list<attribute> _a) {
		for (const auto& at : _a) {
			if (this->attribute_addition_hook(at))
				attributes.insert(at);
		}
		return *this;
	}

	/*! @brief Add attribute(s).
	 * @param[in] _a list of attributes
	 */
	this_type& operator += (attr_list const& _a) {
		for (const auto& at : _a) {
			if (this->attribute_addition_hook(at))
				attributes.insert(at);
		}
		return *this;
	}

	/*! @brief Add a value.
	 * @param[in] _v value
	 */
	this_type& operator += (const string& _v) {
		if (this->data_addition_hook(_v))
			data += _v;
		return *this;
	}
	
	/*! @brief Add value(s).
	 * @param[in] _v {}-list of values
	 */
	this_type& operator += (std::initializer_list<string> _v) {
		for (const auto& vv : _v) {
			if (this->data_addition_hook(vv))
				data += vv;
		}
		return *this;
	}

	/*! @brief Add value(s).
	 * @param[in] _v list of values
	 */
	this_type& operator += (string_list const& _v) {
		for (const auto& vv : _v) {
			if (this->data_addition_hook(vv))
				data += vv;
		}
		return *this;
	}

	//@}

	/*! @brief String cast operator
	 *
	 *  Renders the element, with attributes and pre-rendered data.
	 *  @warning No escaping is done.
	 */
	virtual operator string() const {
		std::basic_stringstream<charT> s;
		s << '<' << this->name;
		for (const auto& a : this->attributes) {
			s << a.first << "=\"" << a.second << "\" ";
		}
		if (this->type == Type::unary) {
			s << " /";
		} else {
			if (this->type == Type::binary) {
				s << '>';
			}
			s << this->data;		
			if (this->type == Type::binary) {
				s << "</" << this->name;
			} else if (this->type == Type::comment) {
				s << "--";
			}
		}
		s << '>';
		return s.str();
	}

	//! Explicitly convert to string (for times when static_cast&lt;charT&gt;(*this) is akward)
	string to_string() const {
		return this->operator string();
	}

protected:
	//! Default constructor for derived classes
	Element()
	: type(0), name(), attributes(), data()
	{ }
	
	/*! @brief Type-exposing constructor for derived classes
	 *  @param[in] type_ Type
	 *  @sa Type
	 */
	Element(unsigned type_)
	: type(type_), name(), attributes(), data()
	{ }


	/*! @brief Hook for attribute addition 
	 * 
	 *  To perform custon behavior (e.g. value checking) on attribute addition,
	 *  override this function in derived classes.
	 *  @retval @c true Add the attribute to the list
	 *  @retval @c false Don't add the attribute to the list
	 */
	virtual bool attribute_addition_hook(const attribute&) { return true; }
	/*! @brief Hook for data addition 
	 *
	 *  To perform custon behavior (e.g. value checking) on data addition,
	 *  override this function in derived classes.
	 *  @retval @c true Add the string to data
	 *  @retval @c false Don't add the string to data
	 */
	virtual bool data_addition_hook(const string&) { return true; }

	//! Element type
	unsigned type;

private:
	//! Element name
	string name;
protected:
	/*! @brief List of attributes.
	 *
	 *  Unused for comment elements.
	 */
	attr_list attributes;
	/*! @brief Pre-rendered embedded data
	 *
	 *  Unused for unary elements.
	 */
	string data;
	
};

/*! @name Concatenators
 *
 * These overloads create new elements which behave as if 
 * attributes and values were appendedto existing elements.
 */
//@{
/*! @brief Embed an element inside another element.
 *  @param[in] e Element
 *  @param[in] e2 Element to embed
 */
template <typename charT, typename T>
typename std::enable_if<std::is_base_of<Element<charT>, T>::value, T>::type
operator + (T e, T&& e2) {
	e += static_cast<std::basic_string<charT>>(e2);
	return e;
}

/*! @brief Concatenate an attribute.
 *  @param[in] e element
 *  @param[in] _a attribute
 */
template <typename charT, typename T>
typename std::enable_if<std::is_base_of<Element<charT>, T>::value, T>::type
operator + (T e, const typename Element<charT>::attribute& _a) {
	e += _a;
	return e;
}

/*! @brief Concatenate attribute(s).
 *  @param[in] e element
 *  @param[in] _a {}-list of attributes
 */
template <typename charT, typename T>
typename std::enable_if<std::is_base_of<Element<charT>, T>::value, T>::type
operator + (T e, std::initializer_list<typename Element<charT>::attribute> _a) {
	e += _a;
	return e;
}

/*! @brief Concatenate attribute(s).
 *  @param[in] e element
 *  @param[in] _a list of attributes
 */
template <typename charT, typename T>
typename std::enable_if<std::is_base_of<Element<charT>, T>::value, T>::type
operator + (T e, typename Element<charT>::attr_list const& _a) {
	e += _a;
	return e;
}

/*! @brief Concatenate a value.
 *  @param[in] e element 
 *  @param[in] _v value
 */
template <typename charT, typename T>
typename std::enable_if<std::is_base_of<Element<charT>, T>::value, T>::type
operator + (T e, const std::basic_string<charT>& _v) {
	e += _v;
	return e;
}
	
/*! @brief Append a rendered element to a string.
 *  @param[in] _v string
 *  @param[in] _e element
 */
template <typename charT, typename T>
typename std::enable_if<std::is_base_of<Element<charT>, T>::value, std::basic_string<charT>>::type
operator + (std::basic_string<charT> const& _v, T&& _e) {
	return _v + static_cast<std::basic_string<charT>>(_e);
}

/*! @brief Concatenate value(s).
 *  @param[in] e element
 *  @param[in] _v {}-list of values
 */
template <typename charT, typename T>
typename std::enable_if<std::is_base_of<Element<charT>, T>::value, T>::type
operator + (T e, std::initializer_list<std::basic_string<charT>> _v) {
	e += _v;
	return e;
}

/*! @brief Concatenate value(s).
 *  @param[in] e element
 *  @param[in] _v list of values
 */
template <typename charT, typename T>
typename std::enable_if<std::is_base_of<Element<charT>, T>::value, T>::type
operator + (T e, typename Element<charT>::string_list const& _v) {
	e += _v;
	return e;
}

//@}

/*! @brief Print to ostream
 *  @note This prints to template ostream, not std::ostream, so
 *  @note Element<T1> cannot print to std::basic_ostream<T2>, if T1 != T2
 *  @param[in] os ostream
 *  @param[in] e element
 */
template <typename charT>
std::basic_ostream<charT>& operator << (std::basic_ostream<charT>& os, const Element<charT>& e) {
	os << static_cast<std::basic_string<charT>>(e);
	return os;
}

/*! @brief HTML begin class
 *
 * This class outputs <!DOCTYPE ...><html ...> when cast to string.
 * @note For XHTML, it adds xmlns and <?xml ...?>.
 * @note To add attributes to the <?xml ?>, add attributes in the form of { "xml=foo", "bar" }.
 * @note Data added is assumed to be valid SGML DTD.
 *
 */
template <typename charT>
class HTML_begin : public virtual Element<charT> {
	typedef HTML_begin<charT> this_type;
	typedef Element<charT> base_type;
public:
	//! Typedef for strings
	typedef typename base_type::string string;
	//! Typedef for attributes
	typedef typename base_type::attribute attribute;
	//! Typedef for attribute lists
	typedef typename base_type::attr_list attr_list;
	//! Typedef for string lists
	typedef typename base_type::string_list string_list;
 
	/*! @brief Create a new HTML start with a given type
	 *  @param[in] type_ HTML type
	 *  @sa doctype::HTML_revision
	 */
	HTML_begin(unsigned type_ = html_doctype::html_revision::xhtml_10_strict)
	: Element<charT>(type_), doctype(html_doctype::html_doctype<charT>(type_)), xml_attributes()
	{
		if (is_xhtml()) {
			this->xml_attributes.insert({ from_pcchar<charT>("version"), from_pcchar<charT>("1") });
			this->attributes.insert({ from_pcchar<charT>("xmlns"), from_pcchar<charT>("http://www.w3.org/1999/xhtml") });
		}
	}

	//! Copy constructor
	HTML_begin(const HTML_begin<charT>& b)
	: Element<charT>(b), doctype(b.doctype)
	{ }

	//! Move constructor
	HTML_begin(HTML_begin<charT>&& b)
	: Element<charT>(std::move(b)), doctype(std::move(b.doctype))
	{ }

	//! Destructor
	virtual ~HTML_begin() { }

	// Copy over all the overloads of () from Element because it's simply too much work to do in-class
	// type_traits magic.
	/*! @name Clone and call
	 *
	 * These overloads create new elements which behave as if attributes and values 
	 * were appended to existing elements.
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
	 *  @param[in] _a {}-list of attributes
	 */
	this_type operator () (std::initializer_list<attribute> _a) const {
		this_type e(*this);
		e += _a;
		return e;
	}
	
	/*! @brief Create a copy of @c *this with given attribute(s).
	 *  @param[in] _a map of attribute
	 */
	this_type operator () (attr_list const& _a) const {
		this_type e(*this);
		e += _a;
		return e;
	}
	
	/*! @brief Create a copy of @c *this with a given value.
	 *  @param[in] _v value
	 */
	this_type operator () (const string& _v) const {
		this_type e(*this);
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c *this with given value(s).
	 *  @param[in] _v {}-list of values
	 */
	this_type operator () (std::initializer_list<string> _v) const {
		this_type e(*this);
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c *this with given value(s).
	 *  @param[in] _v vector of values
	 */
	this_type operator () (string_list const& _v) const {
		this_type e(*this);
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with a given attribute and value.
	 *  @param[in] _a attribute
	 *  @param[in] _v value
	 */
	this_type operator () (const attribute& _a, const string& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}
	
	/*! @brief Create a copy of @c this with a given attribute and value(s).
	 *  @param[in] _a attribute
	 *  @param[in] _v {}-list of values
	 */
	this_type operator () (const attribute& _a, std::initializer_list<string> _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with a given attribute and value(s).
	 *  @param[in] _a attribute
	 *  @param[in] _v list of values
	 */
	this_type operator () (const attribute& _a, string_list const& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with given attribute(s) and a value.
	 *  @param[in] _a {}-list of attributes
	 *  @param[in] _v value
	 */
	this_type operator () (std::initializer_list<attribute> _a, const string& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with given attribute(s) and value(s).
	 *  @param[in] _a {}-list of attributes
	 *  @param[in] _v {}-list of values
	 */
	this_type operator () (std::initializer_list<attribute> _a, std::initializer_list<string> _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with given attribute(s) and value(s).
	 *  @param[in] _a {}-list of attributes
	 *  @param[in] _v list of values
	 */
	this_type operator () (std::initializer_list<attribute> _a, string_list const& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}
	
	/*! @brief Create a copy of @c this with given attribute(s) and a value.
	 *  @param[in] _a map of attributes
	 *  @param[in] _v value
	 */
	this_type operator () (attr_list const& _a, const string& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}

	/*! @brief Create a copy of @c this with given attribute(s) and value(s).
	 *  @param[in] _a list of attributes
	 *  @param[in] _v {}-list of values
	 */
	this_type operator () (attr_list const& _a, std::initializer_list<string> _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}
	/*! @brief Create a copy of @c this with given attribute(s) and value(s).
	 *  @param[in] _a list of attributes
	 *  @param[in] _v list of values
	 */
	this_type operator () (attr_list const& _a, string_list const& _v) const {
		this_type e(*this);
		e += _a;
		e += _v;
		return e;
	}
	//@}

	/*! @brief Overrides Element<T>::operator string () const.
	 *
	 *  Renders all the necessary XML declarations and doctype preamble,
	 *  followed by &lt;html&gt;.
	 *  @sa Element<T>::operator string () const
	 */
	virtual operator string () const {
		std::basic_stringstream<charT> s;
		if (is_xhtml()) {
			s << "<?xml ";
			for (const auto& a : this->xml_attributes) {
				s << a.first << "=\"" << a.second << "\" ";
			}
			s << "?>\r\n";
		}	
		s << this->doctype << "\r\n";
		s << "<html";
		for (const auto& a : this->attributes) {
			s << ' ' << a.first << "=\"" << a.second << '"';
		}
		s << '>';

		return s.str();
	}	
protected:
	/*! @brief Overrides Element<T>::attribute_addition_hook.
	 *
	 * This hook adds support for adding xml attributes in the form of
	 * { "xml=foo", "bar" } in order to get a foo="bar" attribute in
	 * the XML declaration.
	 *
	 * @retval @c true if the attribute is not for XML
	 * @retval @c false if the attribute is for XML
	 */
	virtual bool attribute_addition_hook(const attribute& _a) {
		if (is_xhtml()) {
			if (_a.first == from_pcchar<charT>("lang")) { // make use of lang attribute XHTML-conforming
				this->attributes.insert({ from_pcchar<charT>("xml:lang"), _a.second });
			}
			if (!_a.first.compare(0, 4, from_pcchar<charT>("xml="))) {
				this->xml_attributes.insert(std::make_pair(_a.first.substr(4), _a.second));
				return false;
			}
		}
		return true;
	}
	/*! @brief Overrides Element<T>::data_addition_hook
	 *
	 * Logically, the only complete SGML tag is the DOCTYPE one, so appending data
	 * appends to the doctype. It will not add data past the &lt;html&gt;.
	 *
	 * @retval @c false (don't invoke Element<T>.data.operator +=)
	 */
	virtual bool data_addition_hook(const string& _s) {
		doctype += _s;
		return false;
	}

	//! Doctype
	sgml_doctype::Doctype_declaration<charT> doctype;
	
	//! List of &lt;?xml attributes.
	attr_list xml_attributes;

private:
	bool is_xhtml() const {
		using namespace html_doctype::html_revision;
		return (get_family(this->type) == static_cast<uint8_t>(Family::xhtml));
	}

};

//! This class prints &lt;/html&gt;
template <typename charT>
struct HTML_end {

	HTML_end() { }
	virtual ~HTML_end() { }

	HTML_end const & operator () () const {
		return *this;
	}

	virtual operator std::basic_string<charT> () const {
		return from_pcchar<charT>("</html>");
	}
};

/*! @brief Print to ostream
 *  @note This prints to template ostream, not std::ostream, so
 *  @note HTML_end<T1> cannot print to std::basic_ostream<T2>, if T1 and T2
 *  @note aren't the same type.
 *  @param[in] os ostream
 *  @param[in] e element
 */
template <typename charT>
std::basic_ostream<charT>& operator << (std::basic_ostream<charT>& os, const HTML_end<charT>& e) {
	os << static_cast<std::basic_string<charT>>(e);
	return os;
}

/*! @brief body begin class
 *
 * This class outputs <body ...> when cast to string.
 * @note Unlike HTML_begin<T>, there is no place for valid data here.
 * @note Hence, use of operator() or operator+= with string data shall trigger
 * @note a diagnostic.
 * @note
 * @note In the event that fails, data_addition_hook is overridden to do nothing
 * @note and return false.
 */
template <typename charT>
class Body_begin : public Element<charT> {
	typedef Body_begin<charT> this_type;
public:
	//! Typedef for strings
	typedef typename std::basic_string<charT> string;
	//! Typedef for attributes
	typedef typename std::pair<string, string> attribute;
	//! Typedef for attribute lists
	typedef typename std::map<string, string> attr_list;
 
 	//! Default constructor
	Body_begin()
	: Element<charT>()
	{ }

	//! Copy constructor
	Body_begin(const this_type& b)
	: Element<charT>(b)
	{ }

	//! Move constructor
	Body_begin(this_type&& b)
	: Element<charT>(std::move(b))
	{ }

	//! Destructor
	virtual ~Body_begin() { }

	// Copy over all the overloads of () from Element because it's simply too much work to do in-class
	// type_traits magic.
	/*! @name Clone and call
	 * These overloads create new elements which behave as if attributes and values 
	 * were appended to existing elements.
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
	
	/*! @brief Create a copy of @c *this with given attribute(s).
	 *  @param[in] _a list of attributes
	 */
	this_type operator () (attr_list const& _a) const {
		this_type e(*this);
		e += _a;
		return e;
	}
	//@}
private:
	/* @name Unavailable operators
	 *
	 * These overloaded operators are disabled for semantic reasons.
	 */
	//@{
	// S
	this_type operator () (const string&) const = delete;
	// [S]
	this_type operator () (std::initializer_list<string>) const  = delete;
	this_type operator () (typename Element<charT>::string_list const&) const = delete;
	// AS
	this_type operator () (const attribute&, const string&) const = delete;
	// A[S]
	this_type operator () (const attribute&, std::initializer_list<string>) const = delete;
	this_type operator () (const attribute&, typename Element<charT>::string_list const&) const = delete;
	// [A]S
	this_type operator () (std::initializer_list<attribute>, const string&) const = delete;
	// [A][S]
	this_type operator () (std::initializer_list<attribute>, std::initializer_list<string>) const = delete;
	this_type operator () (std::initializer_list<attribute>, typename Element<charT>::string_list const&) const = delete;
	// [A]S
	this_type operator () (attr_list const& , const string&) const = delete;
	// [A][S]
	this_type operator () (attr_list const&, std::initializer_list<string>) const = delete;
	this_type operator () (attr_list const&, typename Element<charT>::string_list const&) const = delete;


	this_type& operator += (const string&) = delete;
	this_type& operator += (std::initializer_list<string>) = delete;
	this_type& operator += (typename Element<charT>::string_list const&) = delete;
	//@}
public:
	//! Overrides Element<T>::operator string () const
	virtual operator string () const {
		std::basic_stringstream<charT> s;
		s << "<body";
		for (const auto& a : this->attributes) {
			s << ' ' << a.first << "=\"" << a.second << '"';
		}
		s << '>';

		return s.str();
	}

protected:
	// Don't append data
	virtual bool data_addition_hook(const string&) { return false; }
};

//! This class prints &lt;/body&gt;
template <typename charT>
struct Body_end {

	Body_end() { }
	virtual ~Body_end() { }

	Body_end const & operator () () const {
		return *this;
	}

	virtual operator std::basic_string<charT> () const {
		return from_pcchar<charT>("</body>");
	}
};

/*! @brief Print to ostream
 *  @note This prints to template ostream, not std::ostream, so
 *  @note Body_end<T1> cannot print to std::basic_ostream<T2>, if T1 and T2
 *  @note aren't the same type.
 *  @param[in] os ostream
 *  @param[in] e element
 */
template <typename charT>
std::basic_ostream<charT>& operator << (std::basic_ostream<charT>& os, const Body_end<charT>& e) {
	os << static_cast<std::basic_string<charT>>(e);
	return os;
}

/*! @name Attribute taggers
 */
//@{
//! Attribute tagger. Use it to create std::pair&lt;T1,T2&gt;s representing element attributes.
template <typename charT>
std::pair<std::basic_string<charT>, std::basic_string<charT>>
P(const std::basic_string<charT>& s1, const std::basic_string<charT>& s2) {
	return std::make_pair(s1, s2);
}
//! Attribute tagger. Use it to create std::pair&lt;T1,T2&gt;s representing element attributes.
template <typename charT>
std::pair<std::basic_string<charT>, std::basic_string<charT>>
P(std::basic_string<charT>&& s1, std::basic_string<charT>&& s2) {
	return std::make_pair(std::move(s1), std::move(s2));
}
//@}
//! Converts a string literal to std::basic_string
template <typename T, size_t N>
std::basic_string<T> S(const T (&s)[N]) {
	return std::basic_string<T>(s, N);
}

/*! @name Repetition helpers
 *  
 *  A set of functions that repeat an element across a range of values, such that
 *  @code
 *  repeat(e, [a, ] { v_1, ..., v_i, ..., v_n }) => e([a, ] v_1) + ... + e([a, ] v_i) + ... e([a, ] v_n)
 *  @endcode
 *  The overloads are in place to specialize for the type of a, be it none, a pair, or a list of pairs.
 *
 *  @tparam T char type
 *  @param e Element
 *  @param attr Single attribute
 *  @param attrs Attribute list
 *  @param vals Values list
 */
//@{
template <typename T>
std::basic_string<T> repeat(Element<T> const& e,
			    std::initializer_list<typename Element<T>::string> vals)
{
	std::basic_stringstream<T> ss;
	for (const auto& v : vals) {
		ss << e(v);
	}
	return ss.str();
}
template <typename T>
std::basic_string<T> repeat(Element<T> const& e,
			    typename Element<T>::string_list const& vals)
{
	std::basic_stringstream<T> ss;
	for (const auto& v : vals) {
		ss << e(v);
	}
	return ss.str();
}
template <typename T>
std::basic_string<T> repeat(Element<T> const& e,
			    typename Element<T>::attribute const& attr,
			    std::initializer_list<typename Element<T>::string> vals)
{
	std::basic_stringstream<T> ss;
	for (const auto& v : vals) {
		ss << e(attr, v);
	}
	return ss.str();
}
template <typename T>
std::basic_string<T> repeat(Element<T> const& e,
			    typename Element<T>::attribute const& attr,
			    typename Element<T>::string_list const& vals)
{
	std::basic_stringstream<T> ss;
	for (const auto& v : vals) {
		ss << e(attr, v);
	}
	return ss.str();
}
template <typename T>
std::basic_string<T> repeat(Element<T> const& e,
			    std::initializer_list<typename Element<T>::attribute> attrs,
			    std::initializer_list<typename Element<T>::string> vals)
{
	std::basic_stringstream<T> ss;
	for (const auto& v : vals) {
		ss << e(attrs, v);
	}
	return ss.str();
}
template <typename T>
std::basic_string<T> repeat(Element<T> const& e,
			    std::initializer_list<typename Element<T>::attribute> attrs,
			    typename Element<T>::string_list const& vals)
{
	std::basic_stringstream<T> ss;
	for (const auto& v : vals) {
		ss << e(attrs, v);
	}
	return ss.str();
}
template <typename T>
std::basic_string<T> repeat(Element<T> const& e,
			    typename Element<T>::attr_list const& attrs,
			    std::initializer_list<typename Element<T>::string> vals)
{
	std::basic_stringstream<T> ss;
	for (const auto& v : vals) {
		ss << e(attrs, v);
	}
	return ss.str();
}
template <typename T>
std::basic_string<T> repeat(Element<T> const& e,
			    typename Element<T>::attr_list const& attrs,
			    typename Element<T>::string_list const& vals)
{
	std::basic_stringstream<T> ss;
	for (const auto& v : vals) {
		ss << e(attrs, v);
	}
	return ss.str();
}

//@}

}

}

MOSH_FCGI_END


#endif
