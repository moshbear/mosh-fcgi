//! @file header_helpers/status_helper.cpp - String lookup for HTTP status codes
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

#include <string>
#include <stdexcept>
#include <mosh/fcgi/http/helpers/status_helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {

extern "C" const char* status_string(unsigned st) {
	switch (st) {
	// Informational 1xx
	case 100: return "Continue"; // HTTP/1.1
	case 101: return "Switching Protocols"; // HTTP/1.1
	case 102: return "Processing"; // WebDAV (RFC 2518)
	
	// Successful 2xx
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information"; // HTTP/1.1
	case 204: return "No Content"; 
	case 205: return "Reset Content"; 
	case 206: return "Partial Content"; 
	case 207: return "Multi-Status"; // WebDAV (RFC 4918)
	case 208: return "Already Reported"; // WebDAV (RFC 5842)
	case 226: return "IM Used"; // RFC 3229
	
	// Redirection 3xx
	case 300: return "Multiple Choices"; 
	case 301: return "Moved Permanently"; 
	case 302: return "Found"; 
	case 303: return "See Other"; // HTTP/1.1
	case 304: return "Not Modified"; 
	case 305: return "Use Proxy"; // HTTP/1.1
	case 307: return "Temporary Redirect"; // HTTP/1.1
	case 308: return "Resume Incomplete"; // Google Gears

	// Client Error 4xx
	case 400: return "Bad Request"; 
	case 401: return "Unauthorized"; 
	case 402: return "Payment Required"; 
	case 403: return "Forbidden"; 
	case 404: return "Not Found"; 
	case 405: return "Method Not Allowed"; 
	case 406: return "Not Acceptable"; 
	case 407: return "Proxy Authentication Required"; 
	case 408: return "Request Timeout"; 
	case 409: return "Conflict"; 
	case 410: return "Gone"; 
	case 411: return "Length Required"; 
	case 412: return "Precondition Failed"; 
	case 413: return "Request Entity Too Large"; 
	case 414: return "Request-URI Too Long"; 
	case 415: return "Unsupported Media Type"; 
	case 416: return "Requested Range Not Satisfiable"; 
	case 417: return "Expectation Failed"; 
	case 418: return "I'm a teapot"; // RFC 2324
	case 420: return "Insufficient weed"; // Easter egg
	case 422: return "Unprocessable Entity"; // WebDAV (RFC 4918)
	case 423: return "Locked"; // WebDAV (RFC 4918) 
	case 424: return "Failed Dependency"; // WebDAV (RFC 4918) 
	case 425: return "Unordered Collection"; // RFC 3648
	case 426: return "Upgrade Required"; // RFC 2817
	case 428: return "Precondition Required"; // Proposed
	case 429: return "Too Many Requests"; // Proposed
	case 431: return "Request Header Fields Too Large"; // Proposed

	// Server Error 5xx
	case 500: return "Internal Server Error"; 
	case 501: return "Not Implemented"; 
	case 502: return "Bad Gateway"; 
	case 503: return "Service Unavailable"; 
	case 504: return "Gateway Timeout"; 
	case 505: return "HTTP Version Not Supported"; 
	case 506: return "Variant Also Negotiates"; // RFC 2295
	case 507: return "Insufficient Storage"; // WebDAV (RFC 4918) 
	case 508: return "Loop Detected"; // WebDAV (RFC 5842) 
	case 509: return "Bandwidth Limit Exceeded"; // Apache Extension
	case 510: return "Not Extended"; // RFC 2774
	case 511: return "Network Authentication Required"; // Proposed

	default:; // fallthrough to return nullptr
	};
	return nullptr;
}

}

MOSH_FCGI_BEGIN

namespace http {

namespace helpers {

//! HTTP status lookup
namespace status_helper {
	
/*! @brief Get the corresponding status string for a given code
 *  @param[in] st HTTP status code
 *  @return the corresponding status string
 *  @throw std::invalid_argument if the status code is invalid
 */
std::string get_string (unsigned st) {
	const char* str = status_string(st);
	if (str != nullptr) {
		return std::string(str);
	}
	
	throw std::invalid_argument("unassigned status code");
}

} // status_helper

} // helpers

} // http

MOSH_FCGI_END

