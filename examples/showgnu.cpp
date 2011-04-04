/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
* 		2007 Eddie                                                 *
*									   *
* This file is part of fastcgi++.					   *
*									   *
* fastcgi++ is free software: you can redistribute it and/or modify it	   *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.					   *
*									   *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or	   *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public	   *
* License for more details.						   *
*									   *
* You should have received a copy of the GNU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.	   *
****************************************************************************/


#include <fstream>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
// Since we are just displaying an image, we won't need unicode so we don't
// need to use wide characters. We'll keep everything as narrow characters
// and pass the 'char' type along to the Fastcgipp::Request template.

class ShowGnu: public Fastcgipp_m0sh::Request<char>
{
	// Now we define the actual function that sends a response to the client.
	bool response()
	{
		using namespace std;
		using namespace boost;

		// We are going to use boost::posix_time::ptime to communicate
		// the images modification time for cache purposes.
		posix_time::ptime modTime;
		int fileSize;
		int etag;


		{
			// Using the linux stat function (man 2 stat) to get the modification time and filesize beforehand.
			struct stat fileStat;
			stat("gnu.png", &fileStat);
			fileSize = fileStat.st_size;
			modTime = posix_time::from_time_t(fileStat.st_mtime);

			// We'll use the files inode as the etag
			etag = fileStat.st_ino;
		}

		// We need to call this to set a facet in our requests locale regarding how
		// to format the date upon insertion. It needs to conform to the http standard.
		setloc(locale(loc, new posix_time::time_facet("%a, %d %b %Y %H:%M:%S GMT")));

		// If the modification time of the file is older or equal to the if-modified-since value
		// sent to us from the client and we were actually sent an if-modified since value,
		// we don't need to send the image to them.
		int s_etag;
		try { s_etag = lexical_cast<int>(session.environment["HTTP_IF_NONE_MATCH"]); }
		catch(bad_lexical_cast &) { s_etag = 0; }
		posix_time::ptime s_ifmodsince;
		{
			stringstream dateStream;
			boost::shared_ptr<posix_time::time_input_facet> facet(new posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S GMT"));
			dateStream.str(session.environment["HTTP_IF_MODIFIED_SINCE"]);
			dateStream.imbue(locale(locale::classic(), new posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S GMT")));
			dateStream >> s_ifmodsince;
		}

		if(!s_ifmodsince.is_not_a_date_time() && etag==s_etag && modTime<=s_ifmodsince)
		{
			out << "Status: 304 Not Modified\r\n\r\n";
			return true;
		}

		// Setup an fstream for our file.
		std::ifstream image("gnu.png");

		// Now we transmit our HTTP header to the client
		// First we send the modification time of the file
		out << "Last-Modified: " << modTime << '\n';
		// Then a Etag. Note that the session.etag is an integer value. NOT an std::string.
		out << "Etag: " << etag << '\n';
		// Next the size
		out << "Content-Length: " << fileSize << '\n';
		// Then content type. Remember an HTTP header is supposed to be
		// terminated with \r\n\r\n NOT just \n\n
		out << "Content-Type: image/png\r\n\r\n";

		// Now that the header is sent, we can transmit the actual image.
		// To send raw binary data to the client, the streams have a
		// dump function that bypasses the streambuffer and it's code
		// conversion. The function is overloaded to either:
		// out.dump(basic_istream<char>& stream); or
		// out.dump(char* data, size_t size);
		//
		// Remember that if we are using wide characters internally, the stream
		// converts anything sent into the stream to UTF-8 before transmitting
		// to the client. If we want to send binary data, we definitely don't want
		// any code conversion so that is why this function exists.
		out.dump(image);
		
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
		Fastcgipp::Manager<ShowGnu> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
