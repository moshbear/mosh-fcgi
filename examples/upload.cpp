/***************************************************************************
* Copyright (C) 2011 m0shbear <andrey at moshbear dot net>                 *
*               2007 Eddie Carle [eddie@erctech.org]                       *
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
#include <boost/date_time/posix_time/posix_time.hpp>

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/s.hpp>

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
// 1) Be derived from MOSH_FCGI::Request
// 2) Define the virtual response() member function from MOSH_FCGI::Request()

// First things first let's decide on what kind of character set we will use. Let's just
// use good old ASCII this time. No high-bit or wide characters

class Upload: public MOSH_FCGI::Request<char> {
public:
	Upload(): doneHeader(false), totalBytesReceived(0) {}
private:
	// We need to define a state variable so we know where we are when response() is called a second time.
	bool doneHeader;

	void doHeader() {
		using namespace html::element;
		
		// We obviously only want to do our header once.
		if(!doneHeader) {
			out << http::header::content_type("text/html", "US-ASCII");
			
			out << s::html_begin()
			    << s::head({
					s::meta({
						s::P("http-equiv", "Content-Type"),
						s::P("content", "text/html; charset=US-ASCII")
					}),
					s::title("mosh-fcgi: upload progress meter")
				})
			    << s::body_begin();

			doneHeader=true;				
		}
	}

	bool response()
	{
		// In case there was no uploaded data, we need to make our header.
		doHeader();
		
		out << "upload finished";
		out << html::element::s::body_end() << html::element::s::html_end();
		out << "</body></html>";

		// Always return true if you are done. This will let apache know we are done
		// and the manager will destroy the request and free it's resources.
		// Return false if you are not finished but want to relinquish control and
		// allow other requests to operate.
		return true;
	}

	ssize_t totalBytesReceived;
	void in_handler(int bytesReceived) {
		doHeader();
		out << (totalBytesReceived+=bytesReceived) << '/' << session.envs["CONTENT_LENGTH"] << html::element::s::br();
		out.flush();    // Make sure to flush the buffer so it is actually sent.
	}
};

// The main function is easy to set up
int main() {
	try {
		// First we make a Fastcgipp::Manager object, with our request handling class
		// as a template parameter.
		MOSH_FCGI::Manager<Upload> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	} catch(std::exception& e) {
		error_log(e.what());
	}
}
