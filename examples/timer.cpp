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


#include <cstring>
#include <fstream>
#include <memory>

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/bits/u.hpp>

// In this example we are going to use boost::asio to handle our timers and callback.
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace MOSH_FCGI;

boost::asio::io_service io;

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

// No buffering and no form data, so we're deriving Request_base
class Timer: public Request_base {
public:
	Timer(): state(START) {}
private:
	// We need to define a state variable so we know where we are when response() is called a second time.
	enum State { START, FINISH } state;

	std::unique_ptr<boost::asio::deadline_timer> t;

	bool response() {
		switch(state) {
			case START: {
				out << http::header::content_type("text/plain", "US-ASCII");
				
				out << "mosh-fcgi threaded timer\r\n\r\n";
				out << "starting:... \r\n";

				out.flush();

				// Make a five second timer
				t.reset(new boost::asio::deadline_timer(io, boost::posix_time::seconds(5)));

				// Now we work with our callback. Defined in the MOSH_FCGI::Request is a boost::function
				// that takes a MOSH_FCGI::Message (defined in mosh/fcgi/protocol.hpp) as a single argument.
				// This callback function will pass the message on to this request therebye calling the response()
				// function again. The callback function is thread safe. That means you can pass messages back to
				// requests from other threads.

				// Let's build the message we want sent back to here.
				protocol::Message msg;
				// The first part of the message we have to define is the type. A type of 0 means a fastcgi message
				// and is used internally. All other values we can use ourselves to define different message types (sql queries,
				// file grabs, etc...). We will use type=1 for timer stuff.
				msg.type = 1;

				// Now let's put a character string into the message as well. Just for fun.
				{
					char cString[] = "I was passed between two threads!!";
					msg.size = sizeof(cString);
					msg.data.reset(new uchar[sizeof(cString) + 1]);
					std::strncpy(sign_cast<char*>(msg.data.get()), cString, sizeof cString);
				}

				// Now we will give our callback data to boost::asio
				t->async_wait(([this, msg] (const boost::system::error_code&) { this->callback(msg); }));

				// We need to set our state to FINISH so that when this response is called a second time, we don't repeat this.
				state = FINISH;
				
				/* We're not done with the request, so we return false so that the
				 * manager's handler loop can service other requests instead of
				 * just waiting.
				 */
				return false;
			}
			case FINISH: {
				out << "timer finished! message data: \"" << message.data.get() << "\"";
			}
		}
		return true;
	}
};

// The main function is easy to set up
int main() {
	try {
		// Let's first setup a thread for our timers. We assign a work object
		// to it so that boost::asio::io_service::run does not return until
		// the work object goes out of scope.
		boost::asio::io_service::work w(io);
		boost::thread t(boost::bind(&boost::asio::io_service::run, &io));

		// Now we make a MOSH_FCGI::Manager object, with our request handling class
		// as a template parameter.
		MOSH_FCGI::ManagerT<Timer> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
