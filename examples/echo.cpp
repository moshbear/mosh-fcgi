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
#include <boost/date_time/posix_time/posix_time.hpp>

#include <fastcgipp-mosh/request.hpp>
#include <fastcgipp-mosh/manager.hpp>

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
// So, whenever we are going to use UTF-8, our template parameter for Fastcgipp::Request<charT>
// should be wchar_t. Keep in mind that this suddendly makes
// everything wide character and utf compatible. Including HTTP header data (cookies, urls, yada-yada).

class Echo: public Fastcgipp_m0sh::Request<wchar_t>
{
	void dumpfile(Fastcgipp_m0sh::Http::Form::MP_entry<wchar_t, std::vector<char> > const& f) {
		out << "<ul>";
		if (!f.filename.empty())
			out << "<li><b>filename</b>: " << f.filename << "</li>\r\n";
		if (!f.content_type.empty())
			out << "<li><b>content-type</b>: " << f.content_type "</li>\r\n";
		if (!f.headers.empty()) {
			out << "<li><b>headers:</b>: <ul>";
			for (std::map<std::string, std::wstring>::const_iterator jj = f.headers.begin();
				jj != f.headers.end();
				++jj)
			{
				out << "<li><b>" << jj->first << "</b>: " << jj->second << "</li>\r\n";
			}
			out << "</ul></li>\r\n"
		}
		if (!f.data().empty()) {
			out << "<li><b>data</b>: ";
			out.dump(&(f.data()[0]), f.data().size());
			out << "</li>\r\n";
		}
		out << "</ul>\r\n";
	}

	void dumpmixed(Fastcgipp_m0sh::Http::Form::MP_mixed_entry<wchar_t, std::vector<char> > const& m) {
		out << "<ul>";
		if (!f.headers.empty()) {
			out << "<li><b>headers</b>: <ul>";
			for (std::map<std::string, std::wstring>::const_iterator jj = f.headers.begin();
				jj != f.headers.end();
				++jj)
			{
				out << "<li><b>" << jj->first << "</b>: " << jj->second << "</li>\r\n";
			}
			out << "</ul></li>\r\n"
		}
		const std::vector<Fastcgipp_m0sh::Http::Form::MP_entry<wchar_t, std::vector<char> > >& v = f.values;
		if (v.size() > 0) {
			out << "<li><b>files</b>: ";
			if (!v.is_scalar_value()) {
				size_t i_sz = it->second.values().size();
				for (size_t i = 0; i < i_sz; ++i) {
					if (i == 0)
						out << "<ul>";
					out << "<li>";
					dumpfile(it->second.values()[i]);
					out << "</li>\r\n";
					if (i == i_sz - 1)
						out << "</ul><br />\r\n";
				}
			} else 
				dumpfile(it->second.value());
			out << "</ul>\r\n";
		}
		out << "</ul>\r\n";
	}
		
	bool response()
	{
		wchar_t langString[] = { 0x0440, 0x0443, 0x0441, 0x0441, 0x043a, 0x0438, 0x0439, 0x0000 };

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
		for (std::map<std::string, std::string>::const_iterator i = session.environment.begin();
			it != session.environment.end();
			++it
		)
			out << "<li><b>" << it->first << "</b>: "
				<< it->second << "</li>\r\n";
		out << "</ul></p>";
		if (!session.gets.empty()) {
			out << "<h1>GETs</h1><br />\r\n<ul>";
			for (std::map<std::wstring, Fastcgipp_mosh::Http::Form::Entry<wchar_t> >::const_iterator i = session.gets.begin();
				it != session.gets.end();
				++it
			) {
				out << "<li><b>" << it->first << "</b>: ";
				if (!it->second.is_scalar_value()) {
					size_t i_sz = it->second.values().size();
					for (size_t i = 0; i < i_sz; ++i) {
						if (i == 0)
							out << "<ul>";
						out << "<li>" << it->second.values()[i] << "</li>\r\n";
						if (i == i_sz - 1)
							out << "</ul>\r\n";
					}
					out << "</li>";
				}
			}
			out << "</ul><br />\r\n";
		}
		if (!session.posts.empty()) {
			out << "<h1>POSTs</h1> (application/x-www-formurl-encoded)<br />\r\n<ul>";
			for (std::map<std::wstring, Fastcgipp_m0sh::Http::Form::Entry<wchar_t> >::const_iterator i = session.posts.begin();
				it != session.posts.end();
				++it
			) {
				out << "<li><b>" << it->first << "</b>: ";
				if (!it->second.is_scalar_value()) {
					size_t i_sz = it->second.values().size();
					for (size_t i = 0; i < i_sz; ++i) {
						if (i == 0)
							out << "<ul>";
						out << "<li>" << it->second.values()[i] << "</li>\r\n";
						if (i == i_sz - 1)
							out << "</ul><br />\r\n";
					}
				} else
					out << it->second.value();

				out << "</li>\r\n";
			}
		}
			
		if (!session.multipart_posts.empty()) {
			out << "<h1>POSTs</h1> (multipart/form-data)<br />\r\n<ul>";
			for (std::map<std::wstring, Fastcgipp_m0sh::Http::Form::Entry<wchar_t, Fastcgipp_m0sh::Http::Form::MP_entry<wchar_t, std::vector<char> > > >::const_iterator i = session.multipart_posts.begin();
				it != session.multipart_posts.end();
				++it
			) {
				out << "<li><b>" << it->first << "</b>: ";
				if (!it->second.is_scalar_value()) {
					size_t i_sz = it->second.values().size();
					for (size_t i = 0; i < i_sz; ++i) {
						if (i == 0)
							out << "<ul>";
						out << "<li>";
						dumpfile(it->second.values()[i]);
						out << "</li>\r\n";
						if (i == i_sz - 1)
							out << "</ul><br />\r\n";
					}
				} else 
					dumpfile(it->second.value());
				out << "</li>\r\n";

			}
		}
		
		if (!session.multipart_mixed_posts.empty()) {
			out << "<h1>POST files</h1> (multipart/mixed)<br />\r\n<ul>";
			for (std::map<std::wstring, Fastcgipp_m0sh::Http::Form::Entry<wchar_t, Fastcgipp_m0sh::Http::Form::MP_mixed_entry<wchar_t, std::vector<char> > > >::const_iterator i = session.multipart_mixed_posts.begin();
				it != session.multipart_mixed_posts.end();
				++it
			) {
				out << "<li><b>" << it->first << "</b>: ";
				if (!it->second.is_scalar_value()) {
				
					size_t i_sz = it->second.values().size();
					for (size_t i = 0; i < i_sz; ++i) {
						if (i == 0)
							out << "<ul>";
						out << "<li>";
						dumpmixed(it->second.values()[i]);
						out << "</li>\r\n";
						if (i == i_sz - 1)
							out << "</ul><br />\r\n";
					}
					out << "</li>\r\n";
				} else 
					dumpmixed(it->second.value());
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
		Fastcgipp::Manager<Echo> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
