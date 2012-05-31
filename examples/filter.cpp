/***************************************************************************
* Copyright (C) 2011-2 m0shbear                                            *
* Portions  (C) 2007 Eddie Carle                                           *
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

#include <boost/lexical_cast.hpp>
#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/http/misc.hpp>

namespace {
#ifndef UPLOAD_CPP
const bool am_filter = true;
const char* xname = "Filter";
#else
const bool am_filter = false;
const char* xname = "Upload";
#endif
};


using namespace MOSH_FCGI;


// FastCGI-independent error log
void error_log(const char* msg) {
	using namespace std;
	static ofstream error;

	if (!error.is_open()) {
		error.open("/tmp/errlog", ios_base::out | ios_base::app);
	}

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

/*!
 * We're not dealing with form data here, so there's no need to bother with Form_request.
 * We also ignore the received data, so there's no need to bother with buffering.
 * Hence, we derive from MOSH_FCGI::Request_base.
 */
class Filter: public MOSH_FCGI::Request_base {
public:
	
	Filter(): done_header(false), recv_in(false), recv_data(false),
		  in_tot(0), data_tot(0), in_expect(0), data_expect(0)
	{ }
private:
	bool done_header;
	bool recv_in;
	bool recv_data; 


	size_t in_tot;
	size_t data_tot;

	size_t in_expect;
	size_t data_expect;

	bool param_handler(std::pair<std::string, std::string> const& p) {
		if (p.first == "HTTP_CONTENT_LENGTH") {
			try {
				in_expect = boost::lexical_cast<size_t>(p.second);
			} catch (boost::bad_lexical_cast&) {
				in_expect = 0;
			}
		}
		else if (am_filter && p.first == "FCGI_DATA_LENGTH") {
			try {
				data_expect = boost::lexical_cast<size_t>(p.second);
			} catch (boost::bad_lexical_cast&) {
				data_expect = 0;
			}
		}
		/* We don't want to store request parameters, so we
		 * return false in order to instruct Request_base::fill_params
		 * to *not* insert it into the param map
		 */
		return false;
	}

	void do_header() {
		if (done_header)
			return;
		out << http::header::content_type("text/plain", "US-ASCII");
		out << "mosh-fcgi: " << xname << " progress\r\n";
		done_header = true;
	}

	
	bool response() {
		// In case there was no uploaded data, we need to make our header.
		do_header();
		
		if (recv_in) {
			out << "Length check of received IN bytes: ";
			if (in_expect != in_tot) {
				out << "ERROR: expected: " << in_expect << " received: " << in_tot;
			} else {
				out << "ok: " << in_expect;
			}
			out << "\r\n";
		}

		if (am_filter && recv_data) {
			out << "Length check of recieved DATA bytes: ";
			if (data_expect != data_tot) {
				out << "ERROR: expected: " << data_expect << " received: " << data_tot;
			} else {
				out << "ok: " << data_expect;
			}
			out << "\r\n";
		}
		
		out << "Finished.\r\n";

		return true;
	}
	/*
	 * Don't confuse this with void in_handler(size_t).
	 * This version of in_handler handles the entire message, and we can ignore the body if
	 * we so choose, unlike in_handler(size_t), which is the post-parse handler.
	 * This is the pipeline for IN request parsing:
	 *
	 * Request::handler() -> { in_handler(const uchar*, size_t) ;  in_handler(size_t); }
	 * 
	 * So the (const uchar*, size_t) form is the actual handler.
	 * Furthermore, the (size_t) form doesn't even exist unless we're deriving Request or Form_request<charT>.
	 */
	void in_handler(const uchar* data, size_t len) {
		recv_in = true;
		do_header();
		out << "in: " << (in_tot+=len) << '/' << in_expect << "\r\n";
		out.flush();    // Make sure to flush the buffer so it is actually sent.
	}
	/*
	 * Don't confuse this with void data_handler(size_t).
	 *
	 * See above comment for the explanation as to why this form is overriden instead of (size_t).
	 */
	void data_handler(const uchar* data, size_t len) {
		if (!am_filter)
			return;
		recv_data = true;
		do_header();
		out << "data: " << (data_tot+=len) << '/' << data_expect << "\r\n";
		out.flush();    // Make sure to flush the buffer so it is actually sent.
	}
};

// The main function is easy to set up
int main() {
	try {
		// Make a MOSH_FCGI::Manager object, with our handler as the template parameter
		MOSH_FCGI::ManagerT<Filter> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
