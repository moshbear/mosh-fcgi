/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
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

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/ws.hpp>

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

// Let's make our request handling class. It must do the following:
// 1) Be derived from MOSH_FCGI::Request
// 2) Define the virtual response() member function from MOSH_FCGI::Request()

// While native wide utf is used internally, we play around with the external charset; in this case, KOI8-R.

class HelloKoi8r: public MOSH_FCGI::Request<wchar_t> {
	bool response() {
		using namespace MOSH_FCGI::http;
		using namespace MOSH_FCGI::html::element;

		// Print the header. Note use of Fcgistream<charT>::dump(std::string const&) to
		// dump ASCII data directly to the stream.
		// Note: header::operator string() automatically does proper encoding wrt
		// 	line endings, including \r\n\r\n termination
		out.dump(header::content_type("text/html", "KOI8-R"));

		out.output_charset() = "KOI8-R";

		// Here we use the html renderer imported from my fork of mosh-cgi
		out << ws::html_begin();
		out << ws::head({
				ws::meta({
					ws::P("http-equiv", L"Content-Type"),
					ws::P("content", L"text/html"),
					ws::P("charset", L"KOI8-R")
				}),
				ws::title(L"mosh-fcgi: koi8r example")
			});
		out << ws::body_begin();
		out << L"English: Hello, world" << ws::br();
		out << L"Russian: Привет мир" << ws::br();
		out << L"Chinese: ";
		out.flush();
		// Let's see how our iconv wrapper handles EILSEQs
		try {
			out << L"世界您好";
		} catch (std::exception& e) {
			out << L"[exception thrown on charset conversion]";
		}
		out << ws::body_end();
		out << ws::html_end();
		
		// There is also a stream setup for error output. Anything sent here will go
		// to your server's error log. We'll send something there for fun.
		err << L"Hello, error.log from koi8r-test";

		// Always return true if you are done. This will let httpd know we are done
		// and the manager will destroy the request and free it's resources.
		// Return false if you are not finished but want to relinquish control and
		// allow other requests to operate. You might do this after an SQL query
		// while waiting for a reply. Passing messages to requests through the
		// manager is possible but beyond the scope of this example.
		return true;
	}
};

// The main function is easy to set up
int main() {
	try {
		// First we make a MOSH_FCGI::Manager object, with our request handling class
		// as a template parameter.
		MOSH_FCGI::Manager<HelloKoi8r> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	} catch (std::exception& e) {
		// Catch any exception and put them in our errlog file.
		error_log(e.what());
	}
}
