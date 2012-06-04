/***************************************************************************
* Copyright (C) 2012 m0shbear                                              *
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

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/http/misc.hpp>

#include <mosh/fcgi/bits/namespace.hpp>

using namespace MOSH_FCGI;

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
 * Make a class that just dumps all the data, namely formatted FastCGI data, the table of parameters,
 * IN buffer, and DATA buffer.
 */
class Raw_echo: public Request<> {

	bool response()	{
		out << http::header::content_type("text/plain");

		out << "\r\n*** FastCGI ***\r\n";
		out << Request_base::dump();

		out << "\r\n*** PARAMS ***\r\n";
		for (const auto& e : this->envs) {
			// don't chain the following lines into one or the compiler will output a
			// rval ref error due to (at least on gcc-4.6) missing c++11 iostream
			// semantics
			out << '"';
			out << e.first;
			out << "\"->\"";
			out << e.second;
			out << "\"\r\n";
		}
		out << "\r\n*** IN ***\r\n";
		for (const auto& i : post_buf) 
			out << i;
		out << "\r\n*** DATA ***\r\n";
		for (const auto& d : data_buf)
			out << d;
		out << "\r\n";
		
		return true;
	}
};

int main() {
	try {
		// The manager
		MOSH_FCGI::ManagerT<Raw_echo> fcgi;
		// Initialization complete; run the event loop
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
