//TODO: upload filter

//! @file  mosh/fcgi/doc.hpp Documentation file for main page and tutorials
/*!

@version $(VERSION)
@author m0shbear
@author Eddie
@date $(DATE)
@mainpage

@ref intro @n
@ref features @n
@ref overview @n
@ref dep @n
@ref installation @n
@ref tutorials

@section intro Introduction

The mosh-fcgi library is m0shbear's fork of Eddie Carle's fastcgi++ library (which is a
C++ alternative to the FastCGI developer's kit) merged with m0shbear's fork of GNU Cgicc.
Of course, the original pieces of cgicc still in the code are essentially non-existent -
all the non-specifics have been completely rewritten and the original html and http classes
only exist by name, if even that. FastCGI-wise, AUTHORIZER and FILTER roles have been added,
in addition to a complete overhaul of HTTP session parsing, where
application/x-www-formurl-encoded and multipart/mixed support was added, followed by a
full modularization of the parser to the extent that it can even handle "classic" CGI.
Originally,  the FastCGI developer's kit did have a basic C++ interface, but it was
quite limited. Fastcgi++ was a framework which provided an alternative with full C++ semantics.
It can be used to full implement C++ FastCGI web applications. It offered the ability
to work with UTF-8 externally and wide UTF internally, by use of templates. M0shbear's fork
rewrote this to use %iconv, thus giving the freedom to effectively use any IANA-registered
character set for output. The fork also added support for classic CGI, which was removed because
memory management and CPU usage were deemed more important than classic CGI compatibility for C++
web applications, evident with the multiplexed approach utilized by the library.


@section features Features

	@li Support for multiple locales and characters sets, effectively anything supported by %iconv
	@li Internally manages simultaneous requests instead of leaving that to the user
	@li Establishes session data into usable data structures
	@li Implements a task manager that can not only easily communicate outside the library, but with separate threads
	@li Provides a familiar io interface by implementing it through STL iostreams
	@li Complete compliance with FastCGI protocol version 1
	@li Support for %multipart/mixed
	@li Enforcement of aligned data types (while unneccessary on x86, this pedantry ensures portability on PPC and SPARC)

@section overview Overview

Mosh-fcgi, is built around three classes. @c MOSH_FCGI::Manager handles all task
and request management along with the communication inside and outside the
library. Note: this is a base class - use @c ManagerT instead.
@c MOSH_FCGI::Transceiver handles all low level socket io and maintains
send/receive buffers. MOSH_FCGI::Request_base (and derivations thereof in
mosh/fcgi/request.hoo)is designed to handle the individual requests themselves.
The aspects of the FastCGI protocol itself are defined in the MOSH_FCGI::protocol
namespace.

The MOSH_FCGI::Request_base class is a pure virtual base class which handles the
non-streamed portions of FastCGI requests. The class, as is, establishes and parses
FastCGI data. Stream data is parsed in Request<IN_buf, DATA_buf>. Once complete, it
looks to user defined virtual functions for actually generating the response. A
response shall be outputted by the user defined virtuals through an output stream.
Once a request has control over operation, it maintains it until relinquishing it.
Should the user know a request will sit around waiting for data, it can return control
to MOSH_FCGI::Manager and have a message sent back through the manager when the
data is ready. The aspects of the session are built around
MOSH_FCGI::http::session. The HTTP header and HTML generation code imported
from merging the Cgicc fork is also in MOSH_FCGI::http (headers and cookies),
and in MOSH_FCGI::html (HTML, SGML, and XML).

MOSH_FCGI::Manager effectively runs an infinite loop (which can be terminated
through POSIX signals or a function call from another thread) that passes
control to requests that have a message queued for the transceiver. It is smart
enough to go into a sleep mode when there are no tasks to complete or data to
receive.

MOFH_FCGI::Transceiver's transmit half implements a ring buffer that can grow
indefinitely to ensure that operation does not halt. The send half receives full
frames and passes them through MOSH_FCGI::Manager onto the requests. It manages
all the open connections and polls them for incoming data.

@section dep Dependencies

	@li Boost C++ Libraries >1.35.0
	@li Posix compliant OS (socket and thread/lock stuff)
	@li C++11-compliant compiler


@section installation Installation

The installation of mosh-fcgi is pretty standard save a few quirks. The most
basic installation of the library is the traditional:

<tt>tar -xvJf mosh-fcgi-$(VERSION).tar.xz\n
cd mosh_fcgi-$(VERSION)\n
./build.sh\n
make install</tt>

The default prefix for installation is /usr/local. If you wanted to change it
to /usr simply change <tt>"make install"</tt> to <tt>"PREFIX=/usr make install
"</tt>.

@note <tt>make install</tt> automatically manages documentation.
@note
@note Documentation is installed in $PREFIX/share/doc/mosh_fcgi.

For examples, run <tt>make examples</tt>, followed by <tt>make examples-install</tt>.
@warning it will fail if run before <tt>make install</tt>.

Note that it is installed in $WWWROOT/mosh_fcgi.

@section tutorials Tutorials

This is a collection of tutorials that should cover most aspects of the mosh-fcgi library

@subpage hello-world : A simple tutorial outputting "Hello World" in five languages using native @c wchar_t Unicode internally and UTF-8 externally.

@subpage echo : An example of a FastCGI application that echoes all user data and sets a cookie

@subpage raw-echo: An example of a FastCGI application that dumps all the data given, namely the FastCGI request properties, parameter list, and IN and DATA streams

@subpage show-gnu : A tutorial explaining how to display images and non-html data as well as setting locales

@subpage timer : A tutorial covering the use of the task manager and threads to have requests efficiently communicate with non-mosh-fcgi data

@subpage upload : A tutorial covering the use of in_handler to implement a progress meter during uploads

@subpage filter : An example of using the FastCGI data stream 

*/

/*!
@page hello-world Hello World in Five Languages

@section hello-world-tut Tutorial

Our goal here will be to make a FastCGI application that responds to clients
with a "Hello World" in five different languages. 

All code and data is located in the examples directory of the tarball.
Look at the Makefile (examples/Makefile.am) for linking requirements.

\subsection hello-world-err Error Logging

Our first step will be setting up an error logging system. Although requests can
log errors directly to the HTTP server error log, it's nice to have an error
logging system that's separate from the library when testing applications. We 
set up a function that takes a C string and logs it to a file with a timestamp.
Since everyone has access to the /tmp directory, I set it up to send error
messages to /tmp/errlog. You can change it if needed.

@code


#include <fstream>
#include <mosh/fcgi/http/misc.hpp>

void error_log(const char* msg)
{
	using namespace std;

	static ofstream error;

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

@endcode

@subsection hello-world-req Request Handler

Now we need to write the code that actually handles the request. Quite simply,
all we need to do is derive from MOSH_FCGI::Request and define the
MOSH_FCGI::Request_base::response() function. We don't want to store any
parameters sent by the server (since it's going to be ignored), so we override
MOSH_FCGI::Request_base::param_handler and have it return false.
Unicode is handled by Fcgistream, which writes UTF-8 output.

@code
#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/http/header.hpp>

class Hello_world: public MOSH_FCGI::Request_base
{
@endcode

We define a dummy parameter handler in order to tell the library to discard the code.
While you can just leave the argument anonymous, you cannot change the type, since
parameters are stored in a @c std::map .

@code
	bool param_handler(std::pair<std::string, std::string> const&) {
@endcode
	
Nothing is done here and we don't want to store any parameters, so just return false.

@code
		return false;
	}
@endcode


Now we can define our response function. It is this function that is called to
generate a response for the client. While called multiple times, an inline definition
has no performance loss because all the code is virtual calls inside a precompiled
library.

@code
	bool response() {
@endcode

Any data we want to send to the client just gets inserted into the request's
MOSH_FCGI::Fcgistream "out" stream. It works just the same as cout in almost
every way, except that pushing Unicode data involves implicit buffering and
conversion to UTF-8. It also adds the ability to write any of standard or unsigned
@c char strings. We'll start by using MOSH_FCGI::http::header::content_type to output
a Content-type HTTP header to the client. Line endings are handled automatically.

@code
		out << header::content_type("text/html", "utf-8");
@endcode

Now we're ready to insert all the HTML data into the stream. We use @c L"xxx" to
denote wide string literals.

@code
		out << "Hello World in six languages: \r\n\r\n";

		out <<	L"English: Hello World" << "\r\n";
		out <<	L"Russian: Привет мир" << "\r\n";
		out <<	L"Greek: Γεια σας κόσμο" << "\r\n";
		out <<	L"Chinese: 世界您好" << "\r\n";
		out <<	L"Japanese: 今日は世界" << "\r\n";
		out <<	L"Runic English?: ᚺᛖᛚᛟ ᚹᛟᛉᛚᛞ" << "\r\n";
@endcode

We'll also output a little hello to the HTTP server error log just for fun as well.

@code
		err << "Hello, error.log from utf8-test";
@endcode

And we're basically done defining our response! All we need to do is return a
boolean value. Always return @c true if you are done. This will let the manager
know we are done so that post-request cleanup can be performed. Return @c false if
you are not finished but want to relinquish control and allow other requests to
operate. You would do this if the request needed to wait for a message to be passed
back to it through the task manager.

@code
		return true;
	}
};
@endcode

@subsection hello-world-mgr Requests Manager

Now we need to make our main() function. Really all one needs to do is create a
MOSH_FCGI::ManagerT object with the new class we made as a template parameter,
then call its handler. Let's go one step further though and set up a try/catch
in case we get any exceptions and log them with our error_log function.

@code
#include <mosh/fcgi/manager.hpp>
int main() {
	try {
		MOSH_FCGI::ManagerT<HelloWorld> fcgi;
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
@endcode

And that's it! About as simple as it gets.

@section hello-world-code Full Source Code

@code

#include <fstream>

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/http/header.hpp>

void error_log(const char* msg) {
	using namespace std;
	static ofstream error;
	
	if (!error.is_open()) {
		error.open("/tmp/errlog", ios_base::out | ios_base::app);
	}

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

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
		
		err << "Hello, error.log from utf8-test";

		return true;
	}
};

int main() {
	try {
		MOSH_FCGI::ManagerT<Hello_world> fcgi;
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}

@endcode

*/
/*!

@page timer Delayed response

@section timerTutorial Tutorial

Our goal here will be to make a FastCGI application that responds to clients
with some text, waits five seconds and then sends more. We're going to use
threading and boost::asio to handle our timer. You're going to need Boost
C++ libraries for this (at least version 1.35.0).

All code and data is located in the examples directory of the tarball.
Look at the Makefile (examples/Makefile.am) for linking requirements.

@subsection timerError Error Logging

Our first step will be setting up an error logging system. Although requests can
log errors directly to the HTTP server error log, it's nice to have an error
logging system that's separate from the library when testing applications. We 
set up a function that takes a C string and logs it to a file with a timestamp.
Since everyone has access to the /tmp directory, I set it up to send error
messages to /tmp/errlog. You can change it if needed.

@code

#include <fstream>
#include <mosh/fcgi/http/misc.hpp>

void error_log(const char* msg) {
	using namespace std;
	static ofstream error;

	if (!error.is_open()) {
		error.open("/tmp/errlog", ios_base::out | ios_base::app);
	}

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

@endcode

@subsection timerRequest Request Handler

Now we need to write the code that actually handles the request. In this
example we still need to derive from MOSH_FCGI::Request and define the
MOSH_FCGI::Request::response() function, but also some more. We're also
going to need some member data to keep track of our request's execution
state, a default constructor to initialize it and a global
%boost::asio::io_service object. In this example we use classic ASCII, so the
template parameter will be char.

@code
#include <memory>

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/s.hpp>

#include <cstring>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

boost::asio::io_service io;

class Timer: public MOSH_FCGI::Request<char>
{
private:
@endcode

We'll start with our data to keep track of the execution state and a pointer
for our timer object.

@code
	enum State { START, FINISH } state;
	std::unique_ptr<boost::asio::deadline_timer> t;
@endcode

Now we can define our response function. It is this function that is called to
generate a response for the client. We'll start it off with a switch statement
that tests our execution state. It isn't a good idea to define the response()
function inline as it is called from numerous spots, but for the readability of
the example, an exception will be made.

@code
	bool response() {
		switch(state) {
			case START:
			{
@endcode

The first thing that we'll do is output our HTTP header. We let MOSH_FCGI::http::header take care of line endings and other formatting.

@code
				out << MOSH_FCGI::http::header::Header(MOSH_FCGI::http::header::content_type("text/html", "US-ASCII"));
	
@endcode

Then the usual HTML head:

@code
				out << MOSH_FCGI::html::element::s::html_begin()
				    << MOSH_FCGI::html::element::s::head({
						MOSH_FCGI::html::element::s::meta({
							MOSH_FCGI::html::element::s::P("http-equiv", "Content-Type"),
							MOSH_FCGI::html::element::s::P("content", "text/html; charset=US-ASCII")
						}),
						MOSH_FCGI::html::element::s::title("mosh-fcgi: threaded timer")
					})
				    << MOSH_FCGI::html::element::s::body_begin();
@endcode

Output a message saying we are starting the timer

@code
				out << "starting timer..." << MOSH_FCGI::html::element::s::br();
@endcode

If we want to the client to see everything that we just outputted now instead
of when the request is complete, we will have to flush the stream buffer in
its current state.

@code
				out.flush();
@endcode

Now let's make a five second timer.

@code
				t.reset(new boost::asio::deadline_timer(io, boost::posix_time::seconds(5)));
@endcode

Now we work with our MOSH_FCGI::Request::callback. It is a std::function that
takes a MOSH_FCGI::protocol::Message as a single argument. This callback
function will pass the message on to this request thereby having
MOSH_FCGI::Request::response() called again. The callback function is thread
safe. That means you can pass messages back to requests from other threads.

First we'll build the message we want sent back here. Normally the message
would be built by whatever is calling the callback, but this is just a simple
example. A @c type of @c 0 means a FastCGI record and is used internally. All
other values we can use ourselves to define different message types (sql
queries, file grabs, etc...). In this example we will use @c type=1 for timer
stuff.

@code
				MOSH_FCGI::protocol::Message msg;
				msg.type = 1;

				{
					char cString[] = "I was passed between two threads!!";
					msg.size = sizeof(cString);
					msg.data.reset(new char[sizeof(cString) + 1]);
					std::strncpy(msg.data.get(), cString, sizeof(cString));
				}
@endcode

Now we can give our callback function to our timer.

@code
				t->async_wait(boost::bind(callback, msg));
@endcode

We need to set our state to FINISH so that when this response is called a second
time, we don't repeat this.

@code
				state=FINISH;
@endcode

Now we will return and allow the task manager to do other things (or sleep if
there is nothing to do). We must return @c false if the request is not yet complete.

@code
				return false;
			}
@endcode

Next step, we define what's done when MOSH_FCGI::Request::responce() is called a second time.

@code
			case FINISH:
			{
@endcode

Whenever MOSH_FCGI::Request::response() is called, the
MOSH_FCGI::protocol::Message that lead to its calling is stored in
MOSH_FCGI::Request::message.

@code
				out << "timer finished! message data: \"" << message.data.get() << "\"";
				out << MOSH_FCGI::html::element::s::body_end() << MOSH_FCGI::html::element::s::html_end();
@endcode

And we're basically done defining our response! All we need to do is return a
boolean value. Always return @c true if you are done. This will let your httpd

and the manager know we are done so they can destroy the request and free its
resources.

@code
				return true;
			}
		}
	}
@endcode

Now we can define our default constructor.

@code
public:
	Timer(): state(START) {}
};
@endcode

@subsection timerManager Requests Manager

Now we need to make our main() function. In addition to creating a
MOSH_FCGI::Manager object with the new class we made as a template parameter and
calling its handler, we also need to set up our threads and io stuff. This isn't
a boost::asio tutorial, check the boost documentation for clarification on it.

@code
#include <mosh/fcgi/manager.hpp>
int main() {
	try {

		boost::asio::io_service::work w(io);
		boost::thread t(boost::bind(&boost::asio::io_service::run, &io));
		MOSH_FCGI::Manager<Timer> fcgi;
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
@endcode

Clearly, we see that @c MOSH_FCGI::html::element:: is becoming
unnecessarily verbose. Hence, we make use of
@code
	using namespace MOSH_FCGI::html;
@endcode
and
@code
	using namespace html::element;
@endcode

to cut down on the cruft.

(We could certainly hardcode HTML printing here, but it's wise to become
accustomed to MOSH_FCGI::html - it's usefulness will become very useful
in the Echo example.)

@section timerCode Full Source Code

@code

#include <cstring>
#include <fstream>
#include <memory>
#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/s.hpp>
#include <mosh/fcgi/manager.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace MOSH_FCGI;
boost::asio::io_service io;

void error_log(const char* msg)
{
	using namespace std;
	static ofstream error;
	error << '[' << http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

class Timer: public Request<char> {
public:
	Timer(): state(START) {}
private:
	enum State { START, FINISH } state;
	std::unique_ptr<boost::asio::deadline_timer> t;

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
				out << "starting timer..." << s::br();
				out.flush();
				t.reset(new boost::asio::deadline_timer(io, boost::posix_time::seconds(5)));
				protocol::Message msg;
				msg.type = 1;
				{
					char cString[] = "I was passed between two threads!!";
					msg.size = sizeof(cString);
					msg.data.reset(new char[sizeof(cString) + 1]);
					std::strncpy(msg.data.get(), cString, sizeof(cString));
				}
				t->async_wait(boost::bind(callback, msg));
				state=FINISH;
				return false;
			}
			case FINISH:
			{
				out << "timer finished! message data: \"" << message.data.get() << "\"";
				out << s::body_end() << s::html_end();
				return true;
			}
		}
	}
};
int main() {
	try {
		boost::asio::io_service::work w(io);
		boost::thread t(boost::bind(&boost::asio::io_service::run, &io));
		Manager<Timer> fcgi;
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
@endcode

*/

/*!

@page showGnu Display The Gnu

@section showGnuTutorial Tutorial

Our goal here is simple and easy. All we want to do is show the gnu.png file and effectively utilize caching.

All code and data is located in the examples directory of the tarball.
Look at the Makefile (examples/Makefile.am) for linking requirements.

@subsection showGnuError Error Logging

Our first step will be setting up an error logging system. Although requests can
log errors directly to the HTTP server error log, it's nice to have an error
logging system that's separate from the library when testing applications. We 
set up a function that takes a C string and logs it to a file with a timestamp.
Since everyone has access to the /tmp directory, I set it up to send error
messages to /tmp/errlog. You can change it if needed.

@code


#include <fstream>
#include <mosh/fcgi/http/misc.hpp>

void error_log(const char* msg)
{
	using namespace std;

	static ofstream error;

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

@endcode

@subsection showGnuRequest Request Handler

Now we need to write the code that actually handles the request. Quite simply,
all we need to do is derive from MOSH_FCGI::Request and define the
MOSH_FCGI::Request::response() function. Since we're just outputting an image,
we don't need to bother with Unicode and can pass char as the template parameter.

@code
#include <mosh/fcgi/request.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class ShowGnu: public MOSH_FCGI::Request<char>
{
@endcode

Now we can define our response function. It is this function that is called to
generate a response for the client. It's not a good idea to define the
response() function inline as it is called from numerous spots, but for the
example's readability, an exception will be made.

@code
	bool response() {
		using namespace std;
		using namespace boost;
@endcode

We are going to use boost::posix_time::ptime to communicate the image's modification time for caching purposes.

@code
		posix_time::ptime modTime;
		int fileSize;
		int etag;
@endcode

We'll use the POSiX stat() function (man 2 stat) to get the modification time, file size and inode number.

@code
		{
			struct stat fileStat;
			stat("gnu.png", &fileStat);
			fileSize = fileStat.st_size;
			modTime = posix_time::from_time_t(fileStat.st_mtime);
			etag = fileStat.st_ino;
		}
@endcode

We will need to call MOSH_FCGI::Request::setloc() to set a facet in our
request's locale regarding how to format the date upon insertion. It needs to
conform to the HTTP standard. When setting locales for the streams, make sure
to use the MOSH_FCGI::Request::setloc() function instead of directly imbuing
them. This insures that the iconv code still functions properly if used.

@code
		setloc(locale(loc, new posix_time::time_facet("%a, %d %b %Y %H:%M:%S GMT")));
@endcode

Now we get the etag and mod time sent from the client (specifically the value
of the If-None-Match and If-Modified-Since HTTP headers). We fetch the envs
HTTP_IF_NONE_MATCH and IF_MODIFIED_SINCE
from MOSH_FCGI::http::Session_base.envs and cast it to int to
boost::date_time::posix_time:ptime.

@code
		int s_etag;
		try { s_etag = lexical_cast<int>(session.envs["HTTP_IF_NONE_MATCH"]); }
		catch (bad_lexical_cast&) { s_etag = 0; }
		posix_time::ptime s_ifmodsince;
		{
			stringstream dateStream;
			boost::shared_ptr<posix_time::time_input_facet> facet(new posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S GMT"));
			dateStream.str(session.envs["HTTP_IF_MODIFIED_SINCE"]);
			dateStream.imbue(locale(locale::classic(), new posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S GMT")));
			dateStream >> s_ifmodsince;
		}
@endcode


If the modification time of the file is older or equal to the if-modified-since value sent to us from the client and the etag matches, we don't need to send the image to them.

@code
		if (!s_ifmodsince.isnotadatetime() && etag==s_etag && modTime<=s_ifmodsince)
		{
			out << http::header::status(304);
			return true;
		}
#endcode

We're going to use std::fstream to read the file data.

@code
		ifstream image("gnu.png");
@endcode

Now we transmit our HTTP header containing the modification data, file size and etag value.

@code
		{
			http::header::Header h;
			h += {
				http::header::P("Last-Modified", lexical_cast<std::string>(modTime)),
				http::header::P("Etag", lexical_cast<std::string>(etag)),
				http::header::P("Content-Length", lexical_cast<std::string>(fileSize)),
				http::header::P("Content-Type", "image/png")
			};
			out << h;
		}
@endcode

Now that the header is sent, we can transmit the actual image. To send raw
binary data to the client, the streams have a dump function that bypasses the
stream buffer and its code conversion. The function is overloaded to either
MOSH_FCGI::Fcgistream::dump(std::istream& stream),
MOSH_FCGI::Fcgistream::dump(std::string const& str) or
MOSH_FCGI::Fcgistream::dump(char* data, size_t size). Remember that if we are
using wide characters internally, the stream converts anything sent into the
stream to UTF-8 before transmitting to the client. If we want to send binary
data, we definitely don't want any code conversion so that is why this function
exists.

@code
		out.dump(image);
@endcode

And we're basically done defining our response! All we need to do is return a
boolean value. Always return @c true if you are done. This will let httpd and the
manager know we are done so they can destroy the request and free its resources.
Return @c false if you are not finished but want to relinquish control and allow
other requests to operate. You would do this if the request needed to wait for a
message to be passed back to it through the task manager.

@code
		return true;
	}
};
@endcode

@subsection showGnuManager Requests Manager

Now we need to make our main() function. Really all one needs to do is create a
MOSH_FCGI::Manager object with the new class we made as a template parameter,
then call its handler. Let's go one step further though and set up a try/catch
loop in case we get any exceptions and log them with our error_log function.

@code
#include <mosh/fcgi/manager.hpp>
int main() {
	try {
		MOSH_FCGI::Manager<ShowGnu> fcgi;
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
@endcode

@section showGnuCode Full Source Code

@code

#include <fstream>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/s.hpp>

void error_log(const char* msg) {
	using namespace std;
	static ofstream error;

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

class ShowGnu: public MOSH_FCGI::Request<char> {
	bool response() {
		using namespace std;
		using namespace boost;
		using namespace MOSH_FCGI;

		posix_time::ptime modTime;
		int fileSize;
		int etag;

		{
			struct stat fileStat;
			stat("gnu.png", &fileStat);
			fileSize = fileStat.st_size;
			modTime = posix_time::from_time_t(fileStat.st_mtime);
			etag = fileStat.st_ino;
		}

		setloc(locale(loc, new posix_time::time_facet("%a, %d %b %Y %H:%M:%S GMT")));

		int s_etag;
		try { s_etag = lexical_cast<int>(session.envs["HTTP_IF_NONE_MATCH"]); }
		catch (bad_lexical_cast&) { s_etag = 0; }
		posix_time::ptime s_ifmodsince;
		{
			stringstream dateStream;
			boost::shared_ptr<posix_time::time_input_facet> facet(new posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S GMT"));
			dateStream.str(session.envs["HTTP_IF_MODIFIED_SINCE"]);
			dateStream.imbue(locale(locale::classic(), new posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S GMT")));
			dateStream >> s_ifmodsince;
		}
	
		if (!s_ifmodsince.is_not_a_date_time() && etag==s_etag && modTime<=s_ifmodsince) {
			out << http::header::status(304);
			return true;
		}

		{
			http::header::Header h;
			h += {
				http::header::P("Last-Modified", boost::lexical_cast<std::string>(modTime)),
				http::header::P("Etag", boost::lexical_cast<std::string>(etag)),
				http::header::P("Content-Length", boost::lexical_cast<std::string>(fileSize)),
				http::header::P("Content-Type", "image/png")
			};
			out << h;
		}

		std::ifstream image("gnu.png");
		
		out.dump(image);
		
		return true;
	}
};

int main() {
	try {
		MOSH_FCGI::Manager<ShowGnu> fcgi;
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}

@endcode

*/

/*!

@page echo Echo

@section echoTutorial Tutorial

Our goal here will be to make a FastCGI application that responds to clients
with an echo of all session data that was processed. This will include HTTP
header data along with POST data that was transmitted by the client. Since
we want to be able to echo any alphabets, our best solution is to use
UTF-32 wide characters internally and have the library code convert it to
UTF-8 before sending it to the client. 

All code and data is located in the examples directory of the tarball.
Look at the Makefile (examples/Makefile.am) for linking requirements.

@subsection echoError Error Handling

Our first step will be setting up an error logging system. Although requests can
log errors directly to the HTTP server error log, it's nice to have an error
logging system that's separate from the library when testing applications. We 
set up a function that takes a C string and logs it to a file with a timestamp.
Since everyone has access to the /tmp directory, I set it up to send error
messages to /tmp/errlog. You can change it if needed.

@code


#include <fstream>
#include <mosh/fcgi/http/misc.hpp>

void error_log(const char* msg)
{
	using namespace std;

	static ofstream error;

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

@endcode

@subsection echoRequest Request Handler

Now we need to write the code that actually handles the request. Quite simply,
all we need to do is derive from MOSH_FCGI::Request and define the
MOSH_FCGI::Request::response() function. Since we've decided to use wide Unicode
characters, we need to pass wchar_t as the template parameter to the Request
class as opposed to char.

@code
#include <mosh/fcgi/request.hpp>

class HelloWorld: public MOSH_FCGI::Request<wchar_t>
{
@endcode

Because form data can be a multimap, we define a few helpers to make it easier
to echo multipart/form-data and multipart/mixed entries.

@code
	static std::wstring dump_mp(http::form::MP_entry<wchar_t> const& f) {
		ws::Element ul = ws::ul();
		if (f.is_file()) {
			ul += ws::li(ws::b(L"filename")) + S(L": ") + f.filename;
			ul += L"\r\n";
		}
		if (!f.content_type.empty()) {
			ul += MOSH_FCGI::wide_string<wchar_t>(s::li(s::b("content-type")) + S(": ") + f.content_type);
			ul += L"\r\n";
		}
		if (!f.headers.empty()) {
			ws::Element ul2 = ws::ul();
			for (auto& h : f.headers) {
				ul2 += MOSH_FCGI::wide_string<wchar_t>(s::li(s::b(h.first).to_string() + S(": ") + h.second));
				ul2 += L"\r\n";
			}
			ul += ws::li(ws::b(L"headers: ")) + ul2.to_string();
			ul += L"\r\n";
		}
		if (f.is_file()) {
			f.make_file_persistent();
			std::wstringstream wss;
			wss << f.filesize();
			ul += ws::li(ws::b(L"file location: ").to_string() + MOSH_FCGI::wide_string<wchar_t>(f.disk_filename())
					+ ws::br.to_string() + S(L"\r\n")
					+ (ws::b(L"size: ")) + wss.str()
				);
			ul += L"\r\n";
		} else {
			std::wstringstream wss;
			for (auto& g : f.data())
				wss << g;
			ul += ws::li(ws::b(L"data: ")) + wss.str();
			ul += L"\r\n";
		}
		return ul;
	}

	static std::wstring dump_mm(http::form::MP_mixed_entry<wchar_t> const& m) {
		ws::Element ul = ws::ul();
		if (!m.headers.empty()) {
			ws::Element ul2 = ws::ul();
			for (auto& h : m.headers) {
				ul2 += MOSH_FCGI::wide_string<wchar_t>(s::li(s::b(h.first).to_string() + S(": ") + h.second));
				ul2 += L"\r\n";
			}
			ul += ws::li(ws::b(L"headers: ")) + ul2.to_string();
			ul += L"\r\n";
		}
		auto& v = m.values;
		if (v.size() > 0) {
			if (!m.is_scalar_value()) {
				ws::Element ul2 = ws::ul();
				for (const auto& a : v) {
					ul2 += ws::li(dump_mp(a));
					ul2 += L"\r\n";
				}
				ul += ws::li(ws::b(L"files: ")) + ul2.to_string();
			} else if (v.size() == 1) 
				ul += ws::li(ws::b(L"files: ")) + dump_mp(v[0]);
			ul += L"\r\n";
		}
		return ul;
	}
@endcode

Now we can define our response function. It is this function that is called to
generate a response for the client. It isn't a good idea to define the
response() function inline as it is called from numerous spots, but for the
example's readability, an exception will be made.

@code
	bool response()	{
@endcode

First thing we need to do is output our HTTP header. Note that headers must be in ASCII, so we
can't use any Unicode or even high-bit ISO-8859-1 here.

@code
		out << http::header::content_type("text/html", "utf-8") + http::Cookie({"lang", "ru"});
@endcode

Next we'll get some initial HTML stuff out of the way

@code
		out << ws::html_begin();
		out << ws::head({
				ws::meta({
					ws::P("http-equiv", L"Content-Type"),
					ws::P("content", L"text/html"),
					ws::P("charset", L"utf-8")
				}),
				ws::title(L"mosh-fcgi: Echo in UTF-8")
			});
		out << ws::body_begin();
@endcode

Now we are ready to start outputting session data. We'll start with the
non-POST session data, defined in MOSH_FCGI::http::session::Session_base.

@code
		out << ws::h1(L"Session Parameters");
	
		{
			s::Element ul = s::ul();
			for (auto& e : session.envs) {
				ul += s::li(s::b(e.first)) + S(": ") + e.second;
				ul += "\r\n";
			}
@endcode

Because headers are all ASCII, it's safe to dump directly to the ouput stream. The
overhead of ASCII->Unicode->UTF8 is thus eliminated.
			
@code
			out << s::p(ul);
		}
	
@endcode

Now we get to GET data.

@code
		if (!session.gets.empty()) {
			out << s::h1("GETs (decoded QUERY_STRING)").to_string() << s::br.to_string() << "\r\n";
@endcode

We make a temporary MOSH_FCGI::html::Element here in order to make construction 
of the &lt;ul&gt; easier and more readable.

@code
			ws::Element ul = ws::ul();
@endcode

Because GET data is form data, there can be multiple values per name. Hence, we
use an inner loop as needed.

@code
			for (auto& g : session.gets) {
				if (!g.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (const auto& a : g.second.values) {
						ul2 += ws::li(a);
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(g.first)) + S(L": ") + ul2.to_string();
				} else {
					out << ws::li(ws::b(g.first)) + S(L": ") + g.second.value();
				}	
			}
			out << ul << ws::br << L"\r\n";
		}
@endcode

Next, we parse the POST data. Notice the use of the helper function dump_mp  in
simplifying the parsing of multipart/form-data.

@code
		if (!session.posts.empty()) {
			out.dump(s::h1("POSTs").to_string() + s::br.to_string() + "\r\n");
			ws::Element ul = ws::ul();
			for (auto& p : session.posts) {
				if (!p.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (const auto& v : p.second.values) {
						ul2 += ws::li(dump_mp(v));
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(p.first)) + ul2.to_string();
				} else 
					ul += ws::li(ws::b(p.first)) + dump_mp(p.second.value());
			}
			out << ul << ws::br << L"\r\n";
		}
@endcode

Then, we parse multipart/mixed data, if it exists. Notice the use of the
helper function dump_mm in simplifying the parsing of multipart/mixed.

@code

		if (!session.mm_posts.empty()) {
			out.dump(s::h1("POST files (multipart/mixed)").to_string() + s::br.to_string() + "\r\n");
			ws::Element ul = ws::ul();
			for (auto& mm : session.mm_posts) {
				if (!mm.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (const auto& v : mm.second.values) {
						ul2 += ws::li(dump_mm(v));
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(mm.first)) + ul2.to_string();
				} else 
					ul += ws::li(ws::b(mm.first)) + dump_mm(mm.second.value());
			}
			out << ul << ws::br << L"\r\n";
		}
@endcode

Finally, we complete the resulting HTML by printing the closing body and html tags.

@code
		out << ws::body_end << ws::html_end;
@endcode

And we're basically done defining our response! All we need to do is return a
boolean value. Always return @c true if you are done. This will let httpd and the
manager know we are done so they can destroy the request and free it's resources.
Return @c false if you are not finished but want to relinquish control and allow
other requests to operate. You would do this if the request needed to wait for a
message to be passed back to it through the task manager.

@code
		return true;
	}
};
@endcode

@subsection echoManager Requests Manager

Now we need to make our main() function. Really all one needs to do is create a
MOSH_FCGI::Manager object with the new class we made as a template parameter,
then call it's handler. Let's go one step further though and set up a try/catch
in case we get any exceptions and log them with our error_log function.

@code
#include <mosh/fcgi/manager.hpp>

int main() {
	try {
		MOSH_FCGI::Manager<Echo> fcgi;
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
@endcode

@section echoWorldCode Full Source Code

@code
#include <fstream>
#include <algorithm>
#include <string>
#include <sstream>

extern "C" {
#include <unistd.h>
}

#include <mosh/fcgi/bits/t_string.hpp>
#include <mosh/fcgi/http/form.hpp>
#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/ws.hpp>
#include <mosh/fcgi/html/element/s.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

using namespace MOSH_FCGI;
using namespace html::element;

void error_log(const char* msg) {
	using namespace std;
	static ofstream error;
	
	if (!error.is_open()) {
		error.open("/tmp/errlog", ios_base::out | ios_base::app);
	}

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

class Echo: public Request<wchar_t> {
	static std::wstring dump_mp(http::form::MP_entry<wchar_t> const& f) {
		ws::Element ul = ws::ul();
		if (f.is_file()) {
			ul += ws::li(ws::b(L"filename")) + S(L": ") + f.filename;
			ul += L"\r\n";
		}
		if (!f.content_type.empty()) {
			ul += MOSH_FCGI::wide_string<wchar_t>(s::li(s::b("content-type")) + S(": ") + f.content_type);
			ul += L"\r\n";
		}
		if (!f.headers.empty()) {
			ws::Element ul2 = ws::ul();
			for (auto& h : f.headers) {
				ul2 += MOSH_FCGI::wide_string<wchar_t>(s::li(s::b(h.first).to_string() + S(": ") + h.second));
				ul2 += L"\r\n";
			}
			ul += ws::li(ws::b(L"headers: ")) + ul2.to_string();
			ul += L"\r\n";
		}
		if (f.is_file()) {
			f.make_file_persistent();
			std::wstringstream wss;
			wss << f.filesize();
			ul += ws::li(ws::b(L"file location: ").to_string() + MOSH_FCGI::wide_string<wchar_t>(f.disk_filename())
					+ ws::br.to_string() + S(L"\r\n")
					+ (ws::b(L"size: ")) + wss.str()
				);
			ul += L"\r\n";
		} else {
			std::wstringstream wss;
			for (auto& g : f.data())
				wss << g;
			ul += ws::li(ws::b(L"data: ")) + wss.str();
			ul += L"\r\n";
		}
		return ul;
	}

	static std::wstring dump_mm(http::form::MP_mixed_entry<wchar_t> const& m) {
		ws::Element ul = ws::ul();
		if (!m.headers.empty()) {
			ws::Element ul2 = ws::ul();
			for (auto& h : m.headers) {
				ul2 += MOSH_FCGI::wide_string<wchar_t>(s::li(s::b(h.first).to_string() + S(": ") + h.second));
				ul2 += L"\r\n";
			}
			ul += ws::li(ws::b(L"headers: ")) + ul2.to_string();
			ul += L"\r\n";
		}
		auto& v = m.values;
		if (v.size() > 0) {
			if (!m.is_scalar_value()) {
				ws::Element ul2 = ws::ul();
				for (const auto& a : v) {
					ul2 += ws::li(dump_mp(a));
					ul2 += L"\r\n";
				}
				ul += ws::li(ws::b(L"files: ")) + ul2.to_string();
			} else if (v.size() == 1) 
				ul += ws::li(ws::b(L"files: ")) + dump_mp(v[0]);
			ul += L"\r\n";
		}
		return ul;
	}
		
	bool response()	{
		using namespace html::element;
		out.dump(http::header::content_type("text/html", "utf-8") + http::Cookie({"lang", "ru"}));

		out << ws::html_begin();
		out << ws::head({
				ws::meta({
					ws::P("http-equiv", L"Content-Type"),
					ws::P("content", L"text/html"),
					ws::P("charset", L"utf-8")
				}),
				ws::title(L"mosh-fcgi: Echo in UTF-8")
			});
		out << ws::body_begin();
		out << ws::h1(L"Session Parameters");
	
		{
			s::Element ul = s::ul();
			for (auto& e : session.envs) {
				ul += s::li(s::b(e.first)) + S(": ") + e.second;
				ul += "\r\n";
			}
			out.dump(s::p(ul));
		}
	
		if (!session.gets.empty()) {
			out.dump(s::h1("GETs (decoded QUERY_STRING)").to_string() + s::br.to_string() + "\r\n");
			ws::Element ul = ws::ul();
			for (auto& g : session.gets) {
				if (!g.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (const auto& a : g.second.values) {
						ul2 += ws::li(a);
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(g.first)) + S(L": ") + ul2.to_string();
				} else {
					out << ws::li(ws::b(g.first)) + S(L": ") + g.second.value();
				}	
			i}
			out << ul << ws::br << L"\r\n";
		}
		if (!session.posts.empty()) {
			out.dump(s::h1("POSTs").to_string() + s::br.to_string() + "\r\n");
			ws::Element ul = ws::ul();
			for (auto& p : session.posts) {
				if (!p.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (const auto& v : p.second.values) {
						ul2 += ws::li(dump_mp(v));
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(p.first)) + ul2.to_string();
				} else 
					ul += ws::li(ws::b(p.first)) + dump_mp(p.second.value());
			}
			out << ul << ws::br << L"\r\n";
		}
		if (!session.mm_posts.empty()) {
			out.dump(s::h1("POST files (multipart/mixed)").to_string() + s::br.to_string() + "\r\n");
			ws::Element ul = ws::ul();
			for (auto& mm : session.mm_posts) {
				if (!mm.second.is_scalar_value()) {
					ws::Element ul2 = ws::ul();
					for (const auto& v : mm.second.values) {
						ul2 += ws::li(dump_mm(v));
						ul2 += L"\r\n";
					}
					ul += ws::li(ws::b(mm.first)) + ul2.to_string();
				} else 
					ul += ws::li(ws::b(mm.first)) + dump_mm(mm.second.value());
			}
			out << ul << ws::br << L"\r\n";
		}
		out << ws::body_end << ws::html_end;

		return true;
	}
};

int main() {
	try {
		MOSH_FCGI::Manager<Echo> fcgi;
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
@endcode


*/

/*!
@page koi8r Non-UTF8 charset (specifically, KOI8-R)

@section koi8Tutorial Tutorial

Our goal here will be to make a FastCGI application that responds to clients
with a "Hello World" in five different languages. 

All code and data is located in the examples directory of the tarball.
Look at the Makefile (examples/Makefile.am) for linking requirements.

@subsection koi8Error Error Logging

Our first step will be setting up an error logging system. Although requests can
log errors directly to the HTTP server error log, it's nice to have an error
logging system that's separate from the library when testing applications. We 
set up a function that takes a C string and logs it to a file with a timestamp.
Since everyone has access to the /tmp directory, I set it up to send error
messages to /tmp/errlog. You can change it if needed.

@code


#include <fstream>
#include <mosh/fcgi/http/misc.hpp>

void error_log(const char* msg)
{
	using namespace std;

	static ofstream error;

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

@endcode

@subsection koi8Request Request Handler

Now we need to write the code that actually handles the request. Quite simply,
all we need to do is derive from MOSH_FCGI::Request and define the
MOSH_FCGI::Request::response() function. Since we've decided to use wide Unicode
characters, we need to pass @c wchar_t as the template parameter to the Request
class instead of @c char.

@code
#include <mosh/fcgi/request.hpp>

class HelloKoi8r: public MOSH_FCGI::Request<wchart>
{
@endcode

Now we can define our response function. It is this function that is called to
generate a response for the client. It isn't a good idea to define the
response() function inline as it is called from numerous spots, but for the
example's readability, an exception will be made.

@code
	bool response() {
@endcode

Any data we want to send to the client just get's inserted into the request's
MOSH_FCGI::Fcgistream "out" stream. It works just the same as cout in almost
every way. We'll start by using MOSH_FCGI::http::header::Header to output
a HTTP header to the client. Line endings are handled automatically.

@code
		out.dump(header::content_type("text/html", "utf-8"));
@endcode

Now that headers are done, it's time to give out a custom charset. For this, we
use output_charset, which is preferred over setloc, because the charset
conversion uses the value stored by the iconv state rather than the locale.

@note For input charset, look at MOSH_FCGI::http::session::Session_base.charset()

@code
		out.output_charset() = "KOI8-R";
@endcode

Now we're ready to insert all the HTML data into the stream. As with %Echo, we
use @c L"xxx" to denote wide string literals.

@code
		out << ws::html_begin();
		out << ws::head({
				ws::meta({
					ws::P("http-equiv", L"Content-Type"),
					ws::P("content", L"text/html"),
					ws::P("charset", L"KOI8-R")
				}),
				ws::title(L"mosh-fcgi: koi8r example")
			});
		out << ws::body_begin();
		out << L"English: Hello, world" << ws::br();
		out << L"Russian: Привет мир" << ws::br();
		out << L"Chinese: ";
		out.flush();
@endcode

Unlike UTF-8, KOI8-R doesn't support the full Unicode spectrum. Rather, it
only supports Latin and Cyrillic. In order to test how the iconv code handles
EILSEQ, we wrap the attempt with a try-catch and print a failure message when
an exception has been caught.

@code
		try {
			out << L"世界您好";
		} catch (std::exception& e) {
			out << L"[exception thrown on charset conversion]";
		}
@endcode

We continue by completing our output.

@code
		out << ws::body_end();
		out << ws::html_end();
@endcode

We'll also output a little hello to the HTTP server error log just for fun as
well.

@code
		err << L"Hello, error.log from koi8r-test";
@endcode

And we're basically done defining our response! All we need to do is return a
boolean value. Always return @c true if you are done. This will let httpd and
the manager know we are done so they can destroy the request and free its
resources. Return @c false if you are not finished but want to relinquish control
and allow other requests to operate. You would do this if the request needed to
wait for a message to be passed back to it through the task manager.

@code
		return true;
	}
};
@endcode

@subsection koi8rManager Requests Manager

Now we need to make our main() function. Really all one needs to do is create a
MOSH_FCGI::Manager object with the new class we made as a template parameter,
then call its handler. Let's go one step further though and set up a try/catch
in case we get any exceptions and log them with our error_log function.

@code
#include <mosh/fcgi/manager.hpp>
int main() {
	try {
		MOSH_FCGI::Manager<HelloKoi8r> fcgi;
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
@endcode

And that's it! About as simple as it gets.

@section koi8rCode Full Source Code

@code
#include <fstream>

#include <mosh/fcgi/request.hpp>
#include <mosh/fcgi/manager.hpp>
#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/http/header.hpp>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/html/element/ws.hpp>

void error_log(const char* msg) {
	using namespace std;
	static ofstream error;
	
	if (!error.is_open()) {
		error.open("/tmp/errlog", ios_base::out | ios_base::app);
	}

	error << '[' << MOSH_FCGI::http::time_to_string("%Y-%m-%d: %H:%M:%S") << "] " << msg << endl;
}

class HelloKoi8r: public MOSH_FCGI::Request<wchar_t> {
	bool response() {
		using namespace MOSH_FCGI::http;
		using namespace MOSH_FCGI::html::element;

		out.dump(header::content_type("text/html", "KOI8-R"));

		out.output_charset() = "KOI8-R";

		out << ws::html_begin();
		out << ws::head({
				ws::meta({
					ws::P("http-equiv", L"Content-Type"),
					ws::P("content", L"text/html"),
					ws::P("charset", L"KOI8-R")
				}),
				ws::title(L"mosh-fcgi: koi8r example")
			});
		out << ws::body_begin();
		out << L"English: Hello, world" << ws::br();
		out << L"Russian: Привет мир" << ws::br();
		out << L"Chinese: ";
		out.flush();
		try {
			out << L"世界您好";
		} catch (std::exception& e) {
			out << L"[exception thrown on charset conversion]";
		}
		out << ws::body_end();
		out << ws::html_end();
		
		err << L"Hello, error.log from koi8r-test";

		return true;
	}
};

int main() {
	try {
		MOSH_FCGI::Manager<HelloKoi8r> fcgi;
		fcgi.handler();
	} catch (std::exception& e) {
		error_log(e.what());
	}
}
@endcode

*/

