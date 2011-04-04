/***************************************************************************
* Copyright (C) 2007 Eddie Carle [eddie@erctech.org]                       *
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

// First things first let's decide on what kind of character set we will use. Let's just
// use good old ISO-8859-1 this time. No wide characters

class Upload: public Fastcgipp_m0sh::Request<char>
{
public:
	Upload(): doneHeader(false), totalBytesReceived(0) {}
private:
	// We need to define a state variable so we know where we are when response() is called a second time.
	bool doneHeader;

	void doHeader()
	{
		// We obviously only want to do our header once.
		if(!doneHeader)
		{
			// Let's make our header, note the charset=ISO-8859-1. Remember that HTTP headers
			// must be terminated with \r\n\r\n. NOT just \n\n.
			out << "Content-Type: text/html; charset=ISO-8859-1\r\n\r\n";

			// Here it's all stuff you should be familiar with
			out << "<html><head><meta http-equiv='Content-Type' content='text/html; charset=ISO-8859-1' />";
			out << "<title>fastcgi++: Upload Progress Meter</title></head><body>";

			doneHeader=true;				
		}
	}

	bool response()
	{
		// In case there was no uploaded data, we need to make our header.
		doHeader();
		
		out << "Upload Finished!";
		out << "</body></html>";

		// Always return true if you are done. This will let apache know we are done
		// and the manager will destroy the request and free it's resources.
		// Return false if you are not finished but want to relinquish control and
		// allow other requests to operate.
		return true;
	}

	int totalBytesReceived;
	void inHandler(int bytesReceived)
	{
		doHeader();

		out << (totalBytesReceived+=bytesReceived) << '/' << environment().contentLength << "<br />";
		out.flush();    // Make sure to flush the buffer so it is actually sent.
	}
};

// The main function is easy to set up
int main()
{
	try
	{
		// First we make a Fastcgipp::Manager object, with our request handling class
		// as a template parameter.
		Fastcgipp_m0sh::Manager<Upload> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
