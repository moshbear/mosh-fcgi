/***************************************************************************
* Copyright (C) 2011-2 m0shbear                                            *
*		2007 Eddie                                                 *
*                                                                          *
* This file is part of mosh-fcgi.                                          *
*                                                                          *
* mosh-fcgi is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* mosh-fcgi is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with mosh-fcgi.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/

#include <fstream>
#include <algorithm>
#include <string>
#include <sstream>

extern "C" {
#include <unistd.h>
}

#include <mosh/fcgi/bits/t_string.hpp>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/ws.hpp>
#include <mosh/fcgi/html/element/s.hpp>

#include <mosh/fcgi/bits/namespace.hpp>

using namespace MOSH_FCGI;
using namespace html::element;

// I like to have an independent error log file to keep track of exceptions while debugging.
// You might want a different filename. I just picked this because everything has access there.
void error_log(const char* msg) {
	using namespace std;
	static ofstream error;
	
	if (!error.is_open()) {
		error.open("/tmp/errlog", ios_base::out | ios_base::app);
	}

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

/*!
 * Make a request handling class, which does the following:
 * * derive MOSH_FCGI::Form_request, since we're dealing with form data
 * * define the virtual function `bool response()`
 *
 * The character set in use will be Unicode, internally encoded in the native wide Unicode format
 * (e.g. UTF-16 for Windows, UTF-32 for Linux). If you want platform-independent UTF-16 or UTF-32,
 * templatize src/bits/utf8 and instantiate for @c char16_t or @c char32_t .
 *
 * The external encoding is going to be UTF-8. Now, Fcgistream automatically does internal UTF
 * to UTF-8 conversion, but Form_request needs to be instantiated as Form_request<wchar_t> so that
 * the HTTP handler decodes the UTF-8 instead of leaving it unparsed.
 */
class Echo: public Form_request<wchar_t> {
	//! Helper for dumping multipart/form-data entries as HTML
	static std::wstring dump_mp(http::form::MP_entry<wchar_t> const& f) {
		ws::Element ul = ws::ul();
		if (f.is_file()) {
			ul += ws::li(ws::b(L"filename")) + S(L": ") + f.filename;
			ul += L"\r\n";
		}
		if (!f.content_type.empty()) {
			ul += MOSH_FCGI::wide_string<wchar_t>(s::li(s::b("content-type")) + S(": ") + f.content_type);
			ul += L"\r\n";
		}
		if (!f.headers.empty()) {
			ws::Element ul2 = ws::ul();
			for (auto& h : f.headers) {
				ul2 += MOSH_FCGI::wide_string<wchar_t>(s::li(s::b(h.first).to_string() + S(": ") + h.second));
				ul2 += L"\r\n";
			}
			ul += ws::li(ws::b(L"headers: ")) + ul2.to_string();
			ul += L"\r\n";
		}
		if (f.is_file()) {
			f.make_file_persistent();
			std::wstringstream wss;
			wss << f.filesize();
			ul += ws::li(ws::b(L"file location: ").to_string() + MOSH_FCGI::wide_string<wchar_t>(f.disk_filename())
					+ ws::br.to_string() + S(L"\r\n")
					+ (ws::b(L"size: ")) + wss.str()
				);
			ul += L"\r\n";
		} else {
			std::wstringstream wss;
			for (auto& g : f.data())
				wss << g;
			ul += ws::li(ws::b(L"data: ")) + wss.str();
			ul += L"\r\n";
		}
		return ul;
	}

	//! Helper for dumping multipart/mixed entries as HTML
	static std::wstring dump_mm(http::form::MP_mixed_entry<wchar_t> const& m) {
		ws::Element ul = ws::ul();
		if (!m.headers.empty()) {
			ws::Element ul2 = ws::ul();
			for (auto& h : m.headers) {
				ul2 += MOSH_FCGI::wide_string<wchar_t>(s::li(s::b(h.first).to_string() + S(": ") + h.second));
				ul2 += L"\r\n";
			}
			ul += ws::li(ws::b(L"headers: ")) + ul2.to_string();
			ul += L"\r\n";
		}
		auto& v = m.values;
		if (v.size() > 0) {
			if (!m.is_scalar_value()) {
				ws::Element ul2 = ws::ul();
				for (auto& a : v) {
					ul2 += ws::li(dump_mp(a));
					ul2 += L"\r\n";
				}
				ul += ws::li(ws::b(L"files: ")) + ul2.to_string();
			} else if (v.size() == 1) 
				ul += ws::li(ws::b(L"files: ")) + dump_mp(v[0]);
			ul += L"\r\n";
		}
		return ul;
	}
		
	bool response()	{
		using namespace html::element;
		/*
		 * Print a header with a set cookie.
		 * Here, the header is enclosed with parenthese. This is required if you're going to be
		 * concatenating multiple header lines in the form of header_1(...) + ... + header_n(...).
		 * Otherwise, the headers get converted to string before concatenation and the output is not
		 * going to be what you expect. So be safe and parenthesize before stream-shifting.
		 *
		 * Note: Cookie data must be ASCII. Non-ascii can be represented with Q-encoding, Base-64, or
		 * other ASCII-serialization, but the behavior for setting and parsing these cookies is
		 * implementation-defined.
		 */
		out << (http::header::content_type("text/html", "utf-8") + http::Cookie("lang", "ru"));

		/*
		 * Print the html start data.
		 * Here, we use html_begin so that we don't have to print everything to a stream and do
		 * 	out << (s::html() + html_data)	.
		 * 
		 * All HTML elements support the use of { ... ,..., ...} so that (s::e1() + str_1 + ... + str_n)
		 * can be represented instead, as s::e1({ str_1, ..., str_n }). This removes the amount of
		 * parentheses needed and eliminates common potential bugs in string concatenation.
		 *
		 * { ..., ..., ... } has another form, where it accepts an an initializer list of std::pair<s,s>s.
		 * Here, this is a list of element attributes. Using the aforementioned parentheses and
		 * plus-concatenation here is simultaneously tricky, error-prone, ugly, and needlessly complex.
		 * 
		 * Outside the scope of this example is the third form, where each element receives a list of
		 * attributes and a list of values to concatenate within.
		 *
		 * Formally, an Element's argument(s) can be described as:
		 *
		 * (Element)({pair<string,string>|initializer_list<pair<string,string>>}{*}
		 * 		{string | initializer_list<string>}
		 * 	    ),
		 * where {*} is a comma if the argument count is 2,
		 * 		nothing if the argument count is 1, and
		 * 		undefined otherwise.
		 */
		out << s::html_begin();
		out << s::head({
				s::meta({
					s::P("http-equiv", "Content-Type"),
					s::P("content", "text/html"),
					s::P("charset", "utf-8")
				}),
				s::title("mosh-fcgi: Echo in UTF-8")
			});
		// As with html_begin, we use the same logic for body_begin.
		out << s::body_begin();
		out << s::h1("Session Parameters");
	
		// Print env vars
		{
			s::Element ul = s::ul();
			for (auto& e : envs) {
				// Element instances are also appendable, so string data can be added
				// inside an iteration or other unrollable control structure
				ul += s::li(s::b(e.first)) + S(": ") + e.second;
				ul += "\r\n";
			}
			out << s::p(ul);

		}
	
		// Print GETs (i.e. parsed QUERY_STRING)
		if (!session.gets.empty()) {
			out << s::h1("GETs (decoded QUERY_STRING)") << s::br << "\r\n";
			// Here, we use the *wide* string form of Element, so that we can
			// print Unicode data.
			ws::Element ul = ws::ul();
			for (auto& g : session.gets) {
				// Here, we make use of the fact that form data can have multiple
				// values per name
				if (!g.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (auto& a : g.second.values) {
						ul2 += ws::li(a);
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(g.first)) + S(L": ") + ul2.to_string();
				} else {
					// Fcgistream::operator << (Fcgistream&, wstring const&) does
					// UTF-8 decode and buffers the data
					out << ws::li(ws::b(g.first)) + S(L": ") + g.second.value();
				}	
			}
			out << ul << ws::br << L"\r\n";
		}
		// Print POSTDATA
		if (!session.posts.empty()) {
			out << s::h1("POSTs") << s::br << "\r\n";
			ws::Element ul = ws::ul();
			for (auto& p : session.posts) {
				if (!p.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (auto& v : p.second.values) {
						ul2 += ws::li(dump_mp(v));
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(p.first)) + ul2.to_string();
				} else 
					ul += ws::li(ws::b(p.first)) + dump_mp(p.second.value());
			}
			out << ul << ws::br << L"\r\n";
		}
		if (!session.mm_posts.empty()) {
			out << s::h1("POST files (multipart/mixed)") << s::br << "\r\n";
			ws::Element ul = ws::ul();
			for (auto& mm : session.mm_posts) {
				if (!mm.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (auto& v : mm.second.values) {
						ul2 += ws::li(dump_mm(v));
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(mm.first)) + ul2.to_string();
				} else 
					ul += ws::li(ws::b(mm.first)) + dump_mm(mm.second.value());
			}
			out << ul << s::br << L"\r\n";
		}
		// Close the body and html tags
		out << s::body_end << s::html_end;

		/*
		 * We're done here, so we return true, permitting the manager to complete the request.
		 *
		 * If the request is incomplete but is waiting for external data, e.g. a SQL query,
		 * return false so that the manager can deal with other requests.
		 *
		 * Passing messages to requests is possible but beyond the scope of this example.
		 * See timer.cpp for a simple example of intra-request message passing.
		 * 
		 * HTTP sessions support is outside the scope of mosh-fcgi, so cookie handling, etc,
		 * may require a third-party or home-grown mini-library for managing stateful data.
		 */
		return true;
	}
};

// The main function is easy to set up
int main() {
	try {
		// First we make a MOSH_FCGI::Manager object, with our request handling class
		// as a template parameter.
		MOSH_FCGI::ManagerT<Echo> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
