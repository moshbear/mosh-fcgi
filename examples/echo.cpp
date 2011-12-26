/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <sstream>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/ws.hpp>
#include <mosh/fcgi/html/element/s.hpp>

#include <mosh/fcgi/bits/namespace.hpp>

using namespace MOSH_FCGI;

// I like to have an independent error log file to keep track of exceptions while debugging.
// You might want a different filename. I just picked this because everything has access there.
void error_log(const char* msg)
{
	using namespace std;
	using namespace boost;
	static ofstream error;
	if(!error.is_open())
	{
		error.open("/tmp/errlog", ios_base::out | ios_base::app);
		error.imbue(locale(error.getloc(), new posix_time::time_facet()));
	}

	error << '[' << posix_time::second_clock::local_time() << "] " << msg << endl;
}

// Let's make our request handling class. It must do the following:
// 1) Be derived from Fastcgipp::Request
// 2) Define the virtual response() member function from Fastcgipp::Request()

// First things first let's decide on what kind of character set we will use.
// Since we want to be able to echo all languages we will use unicode. The way this
// library handles unicode might be different than some are used to but it is done
// the way it is supposed to be. All internal characters are wide. In this case UTF-32.
// This way we don't have to mess around with variable size characters in our program.
// A string with 10 wchar_ts is ten characters long. Not up in the air as it is with UTF-8.
// Anyway, moving right along, the streams will code convert all the UTF-32 data to UTF-8
// before it is sent out to the client. This way we get the best of both worlds.
//
// So, whenever we are going to use UTF-8, our template parameter for Fastcgipp::Request<char_type>
// should be wchar_t. Keep in mind that this suddendly makes
// everything wide character and utf compatible. Including HTTP header data (cookies, urls, yada-yada).

class Echo: public Request<wchar_t>
{
	static std::wstring dump_mp(http::form::MP_entry<wchar_t> const& f) {
		ws::Element ul = ws::ul();
		if (f.is_file()) {
			ul += ws::li(ws::b(L"filename")) + L": " + f.filename;
			ul += L"\r\n";
		}
		if (!f.content_type.empty()) {
			ul += MOSH_FCGI::wide_string<wchar_t>(s::li(s::b("content-type")) + ": " + f.content_type);
			ul += L"\r\n";
		}
		if (!f.headers.empty()) {
			ws::Element ul2 = ws::ul();
			for (auto& h : f.headers) {
				ul2 += ws::li(MOSH_FCGI::wide_string<wchar_t>((s::b(h.first)) + ": ") + h.second;
				ul2 += L"\r\n";
			}
			ul += ws::li(ws::b(L"headers: ")) + ul2;
			ul += L"\r\n";
		}
		if (f.is_file()) {
			f.make_file_persistent();
			const std::string& s = f.get_disk_filename();
			struct stat sb;
			stat(s.c_str(), &sb);
			ul += ws::li(ws::b(L"file location: ")) + s + ws::br + L"\r\n"
				+ (ws::b(L"size: ")) + boost::lexical_cast<std::wstring>(sb.st_size);
			ul += L"\r\n";
		} else {
			std::wstringstream wss;
			for (auto& g : f.get_data())
				wss << g;
			ul += ws::li(ws::b(L"data: ")) + wss.str();
			ul += L"\r\n";
		}
		return ul;
	}

	static std::wstring dump_mm(http::form::MP_mixed_entry<wchar_t>& m) {
		ws::Element ul = ws::ul();
		if (!m.headers.empty()) {
			ws::Element ul2 = ws::ul();
			for (auto& h : m.headers) {
				ul2 += ws::li(MOSH_FCGI::wide_string<wchar_t>((s::b(h.first)) + ": ") + h.second;
				ul2 += L"\r\n";
			}
			ul += ws::li(ws::b(L"headers: ")) + ul2;
			ul += L"\r\n";
		}
		auto& v = m.values;
		if (v.size() > 0) {
			if (!m.is_scalar_value()) {
				ws::Element ul2 = ws::ul();
				for (const auto& a : v) {
					ul2 += ws::li(dump_mp(a));
					ul2 += L"\r\n";
				}
				ul += ws::li(ws::b(L"files: ")) + ul2;
			} else if (v.size() == 1) 
				ul += ws::li(ws::b(L"files: ")) + dump_mp(v[0]);
			ul += L"\r\n";
		}
		return ul;
	}
		
	bool response()
	{
		using namespace html::element;
		// Print a header with a set cookie
		// Note: Cookie data must be ASCII. Q-encoding or url-encoding can be used, but results
		// 	are implementation definded.
		out.dump(http::header::content_type("text/html", "utf-8") + http::Cookie({"lang", "ru"}));

		out << ws::html_begin();
		out << ws::head({
				ws::meta({
					ws::P("http-equiv", L"Content-Type"),
					ws::P("content", L"text/html"),
					ws::P("charset", L"utf-8")
				}),
				ws::title(L"mosh-fcgi: Hello World in UTF-8")
			});
		out << ws::body_begin();
		out << ws::h1(L"Session Parameters");
	
		// Print env vars
		{
			s::Element ul = ss::ul();
			for (auto& e : session.envs) {
				ul += s::li(s::b(e.first)) + ": " + e.second;
				ul += "\r\n";
			}
			// To avoid the overhead of converting to unicode, then back to UTF-8, we
			// just dump the ascii data as-is
			out.dump(s::p(ul));
		}
	
		// Print GETs (i.e. parsed QUERY_STRING)
		if (!session.gets.empty()) {
			out.dump(s::h1("GETs (decoded QUERY_STRING)").to_string() + s::br.to_string() + "\r\n");
			ws::Element ul = ws::ul();
			for (auto& g : session.gets) {
				if (!g.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (const auto& a : g.second.values) {
						ul2 += ws::li(a);
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(g.first)) + L": " + ul2.to_string();
				} else {
					out << ws::li(ws::b(g.first)) + L": " + g.second.value();
				}	
			}
			out << ul << ws::br << L"\r\n";
		}
		// Print POSTDATA
		if (!session.posts.empty()) {
			out.dump(s::h1("POSTs").to_string() + s::br.to_string() + "\r\n");
			ws::Element ul = ws::ul();
			for (auto& p : session.posts) {
				if (!p.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (const auto& v : p.second.values) {
						ul2 += ws::li(dump_mp(v));
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(p.first)) + ul2;
				} else 
					ul += ws::li(ws::b(p.first)) + dump_mp(p.second.value());
			}
			out << ul << ws::br << L"\r\n";
		}
		if (!session.mm_posts.empty()) {
			out.dump(s::h1("POST files (multipart/mixed)").to_string() + s::br.to_string() + "\r\n");
			ws::Element ul = ws::ul();
			for (auto& mm : session.mm_posts) {
				if (!mm.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (const auto& v : m.second.values) {
						ul2 += ws::li(dump_mm(v);
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(p.first)) + ul2;
				} else 
					ul += ws::li(ws::b(p.first)) + dump_mp(p.second.value());
			}
			out << ul << ws::br << L"\r\n";
		}
		out << ws::body_end() << ws::html_end();

		// Always return true if you are done. This will let http know we are done
		// and the manager will destroy the request and free it's resources.
		// Return false if you are not finished but want to relinquish control and
		// allow other requests to operate. You might do this after an SQL query
		// while waiting for a reply. Passing messages to requests through the
		// manager is possible but beyond the scope of this example.
		return true;
	}
};

// The main function is easy to set up
int main()
{
	try
	{
		// First we make a Fastcgipp::Manager object, with our request handling class
		// as a template parameter.
		MOSH_FCGI::Manager<Echo> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
