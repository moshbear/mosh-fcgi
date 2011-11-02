/***************************************************************************
* Copyright (C) 2007 Eddie                                                 *
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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>

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
		// Let's define our hello worlds. Unfortunately C++ doesn't yet support unicode string literals, but it is
		// just around the corner. Obviously we could have read this data in from a UTF-8 file, but in this example
		// I found it easier to just use these arrays.
		wchar_t russian[] = L"Привет мир";
		wchar_t chinese[] = L"世界您好";
		wchar_t greek[] = L"Γεια σας κόσμο";
		wchar_t japanese[] = L"今日は世界";
		wchar_t runic[] = L"ᚺᛖᛚᛟ ᚹᛟᛉᛚᛞ";

		// Let's make our header, note the charset=utf-8. Remember that HTTP headers
		// must be terminated with \r\n\r\n. NOT just \n\n.
		out << "Content-Type: text/html; charset=utf-8\r\n\r\n";

		// Now it's all stuff you should be familiar with
		out << "<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
		out << "<title>fastcgi++: Hello World in UTF-8</title></head><body>";
		out << "English: Hello World<br>";
		out << "Russian: " << russian << "<br>";
		out << "Greek: " << greek << "<br>";
		out << "Chinese: " << chinese << "<br>";
		out << "Japanese: " << japanese << "<br>";
		out << "Runic English?: " << runic << "<br>";
		out << "</body></html>";

		// There is also a stream setup for error output. Anything sent here will go
		// to your apache error log. We'll send something there for fun.
		err << "Hello apache error log";

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
