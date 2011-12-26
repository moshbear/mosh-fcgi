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


#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/s.hpp>
#include <mosh/fcgi/manager.hpp>

// In this example we are going to use boost::asio to handle our timers and callback.
// Unfortunately because fastcgi buffers the output before sending it to the client by
// default, we will only get to see the true effects of the timer if you put the following
// directive in your apache configuration: FastCgiConfig -flush
#include <cstring>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
boost::asio::io_service io;

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
// use good old ascii this time. No wide or high-bit characters

class Timer: public MOSH_FCGI::Request<char> {
public:
	Timer(): state(START) {}
private:
	// We need to define a state variable so we know where we are when response() is called a second time.
	enum State { START, FINISH } state;

	boost::scoped_ptr<boost::asio::deadline_timer> t;

	bool response() {
		using namespace html::element;
		switch(state) {
			case START: {
				out << http::header::Header(http::header::content_type("text/html", "US-ASCII"));

				out << s::html_begin()
				    << s::head({
						s::meta({
							s::P("http-equiv", "Content-Type"),
							s::P("content", "text/html; charset=US-ASCII")
						}),
						s::title("mosh-fcgi: threaded timer")
					})
				    << s::body_begin();
			
				// Output a message saying we are starting the timer
				out << "starting timer..." << s::br();

				// Let's flush the buffer just to get it out there.
				out.flush();

				// Make a five second timer
				t.reset(new boost::asio::deadline_timer(io, boost::posix_time::seconds(5)));

				// Now we work with our callback. Defined in the MOSH_FCGI::Request is a boost::function
				// that takes a MOSH_FCGI::Message (defined in fastcgi++/protocol.hpp) as a single argument.
				// This callback function will pass the message on to this request therebye calling the response()
				// function again. The callback function is thread safe. That means you can pass messages back to
				// requests from other threads.

				// Let's build the message we want sent back to here.
				MOSH_FCGI::protocol::Message msg;
				// The first part of the message we have to define is the type. A type of 0 means a fastcgi message
				// and is used internally. All other values we can use ourselves to define different message types (sql queries,
				// file grabs, etc...). We will use type=1 for timer stuff.
				msg.type=1;

				// Now let's put a character string into the message as well. Just for fun.
				{
					char cString[] = "I was passed between two threads!!";
					msg.size=sizeof(cString);
					msg.data.reset(new char[sizeof(cString)]);
					std::strncpy(msg.data.get(), cString, sizeof(cString));
				}

				// Now we will give our callback data to boost::asio
				t->async_wait(boost::bind(callback, msg));

				// We need to set our state to FINISH so that when this response is called a second time, we don't repeat this.
				state=FINISH;

				// Now we will return and allow the task manager to do other things (or sleep if there is nothing to do).
				// We must return false if the request is not yet complete.
				return false;
			}
			
			case FINISH:
			{
				// Although we don't need the message we were sent, it is stored in the Request class as member data named
				// "message".
				out << "timer finished! message data: \"" << message.data.get() << "\"";
				out << s::body_end() << s::html_end();

				// Always return true if you are done. This will let httpd know we are done
				// and the manager will destroy the request and free its resources.
				// Return false if you are not finished but want to relinquish control and
				// allow other requests to operate.
				return true;
			}
		}
	}
};

// The main function is easy to set up
int main()
{
	try
	{
		// Let's first setup a thread for our timers. We assign a work object
		// to it so that boost::asio::io_service::run does not return until
		// the work object goes out of scope.
		boost::asio::io_service::work w(io);
		boost::thread t(boost::bind(&boost::asio::io_service::run, &io));

		// Now we make a MOSH_FCGI::Manager object, with our request handling class
		// as a template parameter.
		MOSH_FCGI::Manager<Timer> fcgi;
		// Now just call the object handler function. It will sleep quietly when there
		// are no requests and efficiently manage them when there are many.
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
