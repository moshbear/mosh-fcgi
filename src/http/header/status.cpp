//! @file http/header_helpers/status.cpp - HTTP status codes
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

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <mosh/fcgi/http/header/helper.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/http/header_default.hpp>
#include <src/namespace.hpp>

namespace {

std::string status_string(unsigned st) {
	switch (st) {
#define HTTP_CODE(code, msg) case code : return #code " " msg
	// Informational 1xx
	HTTP_CODE(100, "Continue"); // HTTP/1.1
	HTTP_CODE(101, "Switching Protocols"); // HTTP/1.1
	HTTP_CODE(102, "Processing"); // WebDAV (RFC 2518)
	
	// Successful 2xx
	HTTP_CODE(200, "OK");
	HTTP_CODE(201, "Created");
	HTTP_CODE(202, "Accepted");
	HTTP_CODE(203, "Non-Authoritative Information"); // HTTP/1.1
	HTTP_CODE(204, "No Content"); 
	HTTP_CODE(205, "Reset Content"); 
	HTTP_CODE(206, "Partial Content"); 
	HTTP_CODE(207, "Multi-Status"); // WebDAV (RFC 4918)
	HTTP_CODE(208, "Already Reported"); // WebDAV (RFC 5842)
	HTTP_CODE(226, "IM Used"); // RFC 3229
	
	// Redirection 3xx
	HTTP_CODE(300, "Multiple Choices"); 
	HTTP_CODE(301, "Moved Permanently"); 
	HTTP_CODE(302, "Found"); 
	HTTP_CODE(303, "See Other"); // HTTP/1.1
	HTTP_CODE(304, "Not Modified"); 
	HTTP_CODE(305, "Use Proxy"); // HTTP/1.1
	HTTP_CODE(307, "Temporary Redirect"); // HTTP/1.1
	HTTP_CODE(308, "Resume Incomplete"); // Google Gears

	// Client Error 4xx
	HTTP_CODE(400, "Bad Request"); 
	HTTP_CODE(401, "Unauthorized"); 
	HTTP_CODE(402, "Payment Required"); 
	HTTP_CODE(403, "Forbidden"); 
	HTTP_CODE(404, "Not Found"); 
	HTTP_CODE(405, "Method Not Allowed"); 
	HTTP_CODE(406, "Not Acceptable"); 
	HTTP_CODE(407, "Proxy Authentication Required"); 
	HTTP_CODE(408, "Request Timeout"); 
	HTTP_CODE(409, "Conflict"); 
	HTTP_CODE(410, "Gone"); 
	HTTP_CODE(411, "Length Required"); 
	HTTP_CODE(412, "Precondition Failed"); 
	HTTP_CODE(413, "Request Entity Too Large"); 
	HTTP_CODE(414, "Request-URI Too Long"); 
	HTTP_CODE(415, "Unsupported Media Type"); 
	HTTP_CODE(416, "Requested Range Not Satisfiable"); 
	HTTP_CODE(417, "Expectation Failed"); 
	HTTP_CODE(418, "I'm a teapot"); // RFC 2324
	HTTP_CODE(420, "Insufficient weed"); // Easter egg
	HTTP_CODE(422, "Unprocessable Entity"); // WebDAV (RFC 4918)
	HTTP_CODE(423, "Locked"); // WebDAV (RFC 4918) 
	HTTP_CODE(424, "Failed Dependency"); // WebDAV (RFC 4918) 
	HTTP_CODE(425, "Unordered Collection"); // RFC 3648
	HTTP_CODE(426, "Upgrade Required"); // RFC 2817
	HTTP_CODE(428, "Precondition Required"); // Proposed
	HTTP_CODE(429, "Too Many Requests"); // Proposed
	HTTP_CODE(431, "Request Header Fields Too Large"); // Proposed

	// Server Error 5xx
	HTTP_CODE(500, "Internal Server Error"); 
	HTTP_CODE(501, "Not Implemented"); 
	HTTP_CODE(502, "Bad Gateway"); 
	HTTP_CODE(503, "Service Unavailable"); 
	HTTP_CODE(504, "Gateway Timeout"); 
	HTTP_CODE(505, "HTTP Version Not Supported"); 
	HTTP_CODE(506, "Variant Also Negotiates"); // RFC 2295
	HTTP_CODE(507, "Insufficient Storage"); // WebDAV (RFC 4918) 
	HTTP_CODE(508, "Loop Detected"); // WebDAV (RFC 5842) 
	HTTP_CODE(509, "Bandwidth Limit Exceeded"); // Apache Extension
	HTTP_CODE(510, "Not Extended"); // RFC 2774
	HTTP_CODE(511, "Network Authentication Required"); // Proposed

#undef HTTP_CODE
	default:; // fallthrough to throw
	};
	throw std::invalid_argument("Invalid status code");
}

struct status : public virtual MOSH_FCGI::http::header::Helper {
	Helper::header_pair operator()(unsigned status) {
		return { std::make_pair("Status", status_string(status)) };
	}
};

}
	
SRC_BEGIN

namespace http { namespace header {

Helper_smartptr status() {
	return Helper_smartptr(new ::status);
}

} }

SRC_END

