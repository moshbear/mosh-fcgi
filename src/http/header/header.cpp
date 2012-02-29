//! @file  http/header.cpp HTTP Header
/*
 * Copyright (C) 2012 m0shbear
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA 
 */

#include <initializer_list>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <string>

#include <mosh/fcgi/http/header/header.hpp>
#include <mosh/fcgi/http/header/helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/http/header_default.hpp>
#include <src/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

namespace header {

// ctors
Header::Header(std::shared_ptr<Helper>&& h)
: helper(std::move(h))
{ }

Header::Header(Header const& h)
: headers(h.headers), http_response(h.http_response), helper(h.helper) 
{ }

Header::Header(Header&& h)
: headers(std::move(h.headers)), http_response(std::move(h.http_response)), helper(std::move(h.helper)) 
{ }

// funcall ops

Header Header::operator () (unsigned u) const {
	Header h(*this);
	h += (*helper)(u);
	return h;
}

Header Header::operator () (std::string const& s) const {
	Header h(*this);
	h += (*helper)(s);
	return h;
}

Header Header::operator () (unsigned u, std::string const& s) const {
	Header h(*this);
	h += (*helper)(u, s);
	return h;
}
	
Header Header::operator () (std::string const& s, unsigned u) const {
	Header h(*this);
	h += (*helper)(s, u);
	return h;
}

Header Header::operator () (std::string const& s1, std::string const& s2) const {
	Header h(*this);
	h += (*helper)(s1, s2);
	return h;
}

Header Header::operator () (unsigned u1, unsigned u2) const {
	Header h(*this);
	h += (*helper)(u1, u2);
	return h;
}

// appenders

Header& Header::operator += (const std::pair<std::string, std::string>& p) {
	if (p.first == "@@http_response@@") {
		http_response = p.second;
	} else {
		headers[p.first].push_back(p.second);
	}
	return *this;
}

Header& Header::operator += (std::initializer_list<std::pair<std::string, std::string>> hp) {
	for (const auto& p : hp) {
		operator += (p);
	}
	return *this;
}

Header& Header::operator += (std::vector<std::pair<std::string, std::string>> const& hv) {
	for (const auto& p : hv) {
		operator += (p);
	}
	return *this;
}

Header::operator std::string () const {
	std::stringstream ss;
	if (!http_response.empty()) {
		ss << http_response << "\r\n";
	}
	for (const auto& h_k : headers) {
		for (const auto& h_v: h_k.second)  {
			ss << h_k.first << ": " << h_v << "\r\n";
		}
	}
	ss << "\r\n";
	return ss.str();
}

// concatenators

Header operator + (const Header& _h, const std::pair<std::string, std::string>& _p) {
	Header h(_h);
	h += _p;
	return h;
}
	
Header operator + (Header&& _h, const std::pair<std::string, std::string>& _p) {
	Header h(_h);
	h += _p;
	return std::move(h);
}
	
Header operator + (const Header& _h, std::initializer_list<std::pair<std::string, std::string>> _hp) {
	Header h(_h);
	h += _hp;
	return h;
}
	
Header operator + (Header&& _h, std::initializer_list<std::pair<std::string, std::string>> _hp) {
	Header h(std::move(_h));
	h += _hp;
	return std::move(h);
}	

std::ostream& operator << (std::ostream& os, const Header& h) {
	os << static_cast<std::string>(h);
	return os;
}

std::pair<std::string, std::string> P(std::string&& s1, std::string&& s2) {
	return std::make_pair(std::move(s1), std::move(s2));
}
std::pair<std::string, std::string> P(std::string const& s1, std::string const& s2) {
	return std::make_pair(s1, s2);
}



// constants

const Header content_type (SRC::http::header::content_type());
const Header redirect (SRC::http::header::redirect());
const Header response (SRC::http::header::response());
const Header status (SRC::http::header::status());
const Header x_sendfile (SRC::http::header::x_sendfile());
}

}

MOSH_FCGI_END
