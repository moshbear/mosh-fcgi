/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*               2007 Eddie                                                 *
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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/ws.hpp>

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
// Obviously with all these different languages we can't use something like
// ISO-8859-1. Our only option is unicode and in particular UTF-8. The way this
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

class HelloWorld: public MOSH_FCGI::Request<wchar_t>
{
	bool response()
	{
		using namespace MOSH_FCGI::http;
		using namespace MOSH_FCGI::html::element;

		// Print the header. Note use of Fcgistream<charT>::dump(std::string const&) to
		// dump ASCII data directly to the stream.
		// Note: header::operator string() automatically does proper encoding wrt
		// 	line endings, including \r\n\r\n termination
		out.dump(header::content_type("text/html", "utf-8"));


		// Here we use the html renderer imported from my fork of mosh-cgi
		out << ws::html_begin();
		out << ws::head({
				ws::meta({
					ws::P("http-equiv", L"Content-Type"),
					ws::P("content", L"text/html"),
					ws::P("charset", L"utf-8")
				}),
				ws::title(L"mosh-fcgi: Hello World in UTF-8")
			});
		out << ws::body({
			L"English: Hello World", ws::br,
			L"Russian: Привет мир", ws::br,
			L"Greek: Γεια σας κόσμο", ws::br,
			L"Chinese: 世界您好", ws::br,
			L"Japanese: 今日は世界", ws::br,
			L"Runic English?: ᚺᛖᛚᛟ ᚹᛟᛉᛚᛞ", ws::br
		});
		out << ws::html_end();
		
		// There is also a stream setup for error output. Anything sent here will go
		// to your server's error log. We'll send something there for fun.
		err << L"Hello, error.log from utf8-test";

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
		MOSH_FCGI::Manager<HelloWorld> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		// Catch any exception and put them in our errlog file.
		error_log(e.what());
	}
}
