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

#include <fstream>

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/http/header.hpp>

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
 * Given that Fcgistream converts UTF-[wchar] to UTF-8, we're not dealing with UTF-8 input,
 * and we're not reading IN or DATA, we don't need to derive from anything fancier than Request_base.
 * Furthermore, we're not reading PARAMS, so we can virtually override params_handler to return false on
 * all input.
 */
class Hello_world: public MOSH_FCGI::Request_base {
	bool param_handler(std::pair<std::string, std::string> const&) {
		return false;
	}
	bool response() {
		using namespace MOSH_FCGI::http;

		out << header::content_type("text/plain", "utf-8");

		out << "Hello World in six languages: \r\n\r\n";

		out <<	L"English: Hello World" << "\r\n";
		out <<	L"Russian: Привет мир" << "\r\n";
		out <<	L"Greek: Γεια σας κόσμο" << "\r\n";
		out <<	L"Chinese: 世界您好" << "\r\n";
		out <<	L"Japanese: 今日は世界" << "\r\n";
		out <<	L"Runic English?: ᚺᛖᛚᛟ ᚹᛟᛉᛚᛞ" << "\r\n";
		
		// There is also a stream setup for error output. Anything sent here will go
		// to your server's error log. We'll send something there for fun.
		err << "Hello, error.log from utf8-test";

		// We're done here
		return true;
	}
};

// The main function is easy to set up
int main() {
	try {
		// First we make a MOSH_FCGI::Manager object, with our request handling class
		// as a template parameter.
		MOSH_FCGI::ManagerT<Hello_world> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	} catch (std::exception& e) {
		// Catch any exception and put them in our errlog file.
		error_log(e.what());
	}
}
