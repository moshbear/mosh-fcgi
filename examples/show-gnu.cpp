/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
* 		2007 Eddie                                                 *
*									   *
* This file is part of mosh-fcgi.					   *
*									   *
* mosh-fcgi is free software: you can redistribute it and/or modify it	   *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.					   *
*									   *
* mosh-fcgi is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or	   *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public	   *
* License for more details.						   *
*									   *
* You should have received a copy of the GNU Lesser General Public License *
* along with mosh-fcgi.  If not, see <http://www.gnu.org/licenses/>.	   *
****************************************************************************/


#include <fstream>
#include <sstream>
extern "C" {
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

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

// Derive from Request_base because we're not going to be processing the IN or DATA streams
class Show_gnu: public MOSH_FCGI::Request_base {
	// Now we define the actual function that sends a response to the client.
	bool response() {
		using namespace std;
		using namespace boost;
		using namespace MOSH_FCGI;

		time_t modTime;
		int fileSize;
		int etag;

		{
			// stat the files and copy the useful information to the aforedeclared modTime, fileSize, etag
			struct stat fileStat;
			stat("gnu.png", &fileStat);
			fileSize = fileStat.st_size;
			modTime = fileStat.st_mtime;

			// We'll use the files inode as the etag
			etag = fileStat.st_ino;
		}

		/*
		 * Check to see if the client's version of the file matches ours (i.e. the etags match)
		 * and that the modification time is lesser or equal.
		 *
		 * If so, then, don't bother sending and just print a 304 status header.
		 */
		int s_etag;
		try { s_etag = lexical_cast<int>(envs["HTTP_IF_NONE_MATCH"]); }
		catch (bad_lexical_cast&) { s_etag = 0; }
		posix_time::ptime s_ifmodsince;
		{
			stringstream dateStream;
			boost::shared_ptr<posix_time::time_input_facet> facet(new posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S GMT"));
			dateStream.str(envs["HTTP_IF_MODIFIED_SINCE"]);
			dateStream.imbue(locale(locale::classic(), new posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S GMT")));
			dateStream >> s_ifmodsince;
		}
	
		if (!s_ifmodsince.is_not_a_date_time() && etag==s_etag && posix_time::from_time_t(modTime)<=s_ifmodsince) {
			out << http::header::status(304);
			return true;
		}

		{
			out << (http::header::content_type("image/png")
				// First the modification time of the file
				+ http::header::P("Last-Modified", ([&modTime](){
									struct tm* _t = gmtime(&modTime);
									std::unique_ptr<struct tm> t(new struct tm);
									memcpy(t.get(), _t, sizeof(struct tm));
									return http::time_to_string("%a, %d %b %Y %H:%M:%S GMT", *t);
								})())
				// Then a Etag. Note that the session.etag is an integer value. NOT an std::string.
				+ http::header::P("Etag", boost::lexical_cast<std::string>(etag))
				// Next the size
				+ http::header::P("Content-Length", boost::lexical_cast<std::string>(fileSize))
			);
		}

		/*
		 * Do an unbuffered send of the image data by constructing an ifstream and then calling
		 * Fcgistream::dump().
		 */
		std::ifstream image("gnu.png");
		out.dump(image);
		
		return true;
	}
};

// The main function is easy to set up
int main() {
	try {
		// Make a MOSH_FCGI::Manager object, with the handler class as the template parameter.
		MOSH_FCGI::ManagerT<Show_gnu> fcgi;
		// Initiate the event loop
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
