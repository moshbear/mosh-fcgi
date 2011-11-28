/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*		2007 Eddie                                                 *
*                                                                          *
* This file is part of fastcgi++.                                          *
*                                                                          *
* fastcgi++ is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/

#include <fstream>
#include <algorithm>
#include <boost/date_time/posix_time/posix_time.hpp>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>

#include <mosh/fcgi/bits/namespace.hpp>

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

class Echo: public MOSH_FCGI::Request<wchar_t>
{
	void dump_mp(MOSH_FCGI::http::form::MP_entry<wchar_t> const& f) {
		out << "<ul>";
		if (f.is_file())
			out << "<li><b>filename</b>: " << f.filename << "</li>\r\n";
		if (!f.content_type.empty())
			out << "<li><b>content-type</b>: " << f.content_type.c_str() << "</li>\r\n";
		if (!f.headers.empty()) {
			out << "<li><b>headers:</b>: <ul>";
			for (auto& h : f.headers) {
				out << "<li><b>" << h.first.c_str() << "</b>: "
				    << h.second << "</li>\r\n";
			}
			out << "</ul></li>\r\n";
		}
		if (f.is_file()) {
			f.make_file_persistent();
			const std::string& s = f.get_disk_filename();
			out << "<li><b>file location</b>: " << s.c_str() << "<br />\r\n";
			struct stat sb;
			stat(s.c_str(), &sb);
			out << "<b>size</b>: " << sb.st_size << "</li>\r\n";
		} else {		
			out << "<li><b>data</b>: ";
			for (auto& g : f.get_data())
				out << g;
			out << "</li>\r\n";
		}
		out << "</ul>\r\n";
	}

	void dump_mm(MOSH_FCGI::http::form::MP_mixed_entry<wchar_t>& m) {
		out << "<ul>";
		if (!m.headers.empty()) {
			out << "<li><b>headers</b>: <ul>";
			for (auto& h : m.headers) {
				out << "<li><b>" << h.first.c_str() << "</b>: "
				    << h.second << "</li>\r\n";
			}
			out << "</ul></li>\r\n";
		}
		auto& v = m.values;
		if (v.size() > 0) {
			out << "<li><b>files</b>: ";
			if (!m.is_scalar_value()) {
				size_t i_sz = v.size();
				for (size_t i = 0; i < i_sz; ++i) {
					if (i == 0)
						out << "<ul>";
					out << "<li>";
					dump_mp(v[i]);
					out << "</li>\r\n";
					if (i == i_sz - 1)
						out << "</ul><br />\r\n";
				}
			} else if (v.size() == 1)
				dump_mp(v[0]);
			out << "</ul>\r\n";
		}
		out << "</ul>\r\n";
	}
		
	bool response()
	{
		wchar_t langString[] = L"русский";

		// Let's make our header, note the charset=utf-8. Remember that HTTP headers
		// must be terminated with \r\n\r\n. NOT just \n\n.
		// Let's set a cookie just for fun too, in UTF-8.
		out << "Set-Cookie: lang=" << langString << '\n';
		out << "Content-Type: text/html; charset=utf-8\r\n\r\n";

		// Now it's all stuff you should be familiar with
		out << "<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
		out << "<title>fastcgi++-m0sh: Echo in UTF-8</title></head><body>";

		out << "<h1>Session Parameters</h1>";
		out << "<p><ul>";
		for (auto& e : session.envs) {
			out << "<li><b>" << e.first.c_str() << "</b>: "
				<< e.second.c_str() << "</li>\r\n";
		}
		out << "</ul></p>";
		if (!session.gets.empty()) {
			out << "<h1>GETs</h1><br />\r\n<ul>";
			for (auto& g : session.gets) {
				out << "<li><b>" << g.first << "</b>: ";
				if (!g.second.is_scalar_value()) {
					size_t i_sz = g.second.values.size();
					for (size_t i = 0; i < i_sz; ++i) {
						if (i == 0)
							out << "<ul>";
						out << "<li>" << g.second.values[i] << "</li>\r\n";
						if (i == i_sz - 1)
							out << "</ul>\r\n";
					}
				} else
					out << g.second.value();
				out << "</li>";
					
			}
			out << "</ul><br />\r\n";
		}
		if (!session.posts.empty()) {
			out << "<h1>POSTs</h1><br />\r\n<ul>";
			for (auto& p : session.posts) {
				out << "<li><b>" << p.first << "</b>: ";
				if (!p.second.is_scalar_value()) {
					size_t i_sz = p.second.values.size();
					for (size_t i = 0; i < i_sz; ++i) {
						if (i == 0)
							out << "<ul>";
						out << "<li>";
						dump_mp(p.second.values[i]);
						out << "</li>\r\n";
						if (i == i_sz - 1)
							out << "</ul><br />\r\n";
					}
				} else 
					dump_mp(p.second.value());
				out << "</li>\r\n";

			}
		}
		
		if (!session.mm_posts.empty()) {
			out << "<h1>POST files</h1> (multipart/mixed)<br />\r\n<ul>";
			for (auto& mm : session.mm_posts) {
				out << "<li><b>" << mm.first << "</b>: ";
				if (!mm.second.is_scalar_value()) {
					size_t i_sz = mm.second.values.size();
					for (size_t i = 0; i < i_sz; ++i) {
						if (i == 0)
							out << "<ul>";
						out << "<li>";
						dump_mm(mm.second.values[i]);
						out << "</li>\r\n";
						if (i == i_sz - 1)
							out << "</ul><br />\r\n";
					}
					out << "</li>\r\n";
				} else if (mm.second.values.size() == 0)
					dump_mm(mm.second.values[0]);
				out << "</li>\r\n";
			}
			out << "</ul>\r\n";
		}
		out << "</body></html>";

		// Always return true if you are done. This will let apache know we are done
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
