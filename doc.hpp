//! \file doc.hpp Documentation file for main page and tutorials
/*!

\version $(VERSION)
\author Eddie
\date $(DATE)
\mainpage

\ref intro \n
\ref features \n
\ref overview \n
\ref dep \n
\ref installation \n
\ref tutorials

\section intro Introduction

The fastcgi++ library started out as a C++ alternative to the official FastCGI developers kit. Although the official developers kit provided some degree of C++ interface, it was very limited. The goal of this project was to provide a framework that offered all the facilities that the C++ language has to offer. Over time the scope broadened to the point that it became more than just a simple protocol library, but a platform to develop web application under C++. To the dismay of many, this library has zero support for the old CGI protocol. The consensus was that if one were to be developing web applications under C++, efficient memory management and CPU usage would be a top priority, not CGI compatibility. Effective management of simultaneous requests without the need for multiple threads is something that fastcgi++ does best. Session data is organized into meaningful data types as opposed to a series of text strings. Internationalization and Unicode support is another top priority. The library is templated to allow internal wide character use for efficient text processing while code converting down to utf-8 upon transmission to the client.

\section features Features

	\li Support for multiple locales and characters sets including wide Unicode and utf-8
	\li Internally manages simultaneous requests instead of leaving that to the user
	\li Establishes session data into usable data structures
	\li Implements a task manager that can not only easily communicate outside the library, but with separate threads
	\li Provides a familiar io interface by implementing it through STL iostreams
	\li Complete compliance with FastCGI protocol version 1

\section overview Overview

The fastcgi++ library is built around three classes. Fastcgipp::Manager handles all task and request management along with the communication inside and outside the library. Fastcgipp::Transceiver handles all low level socket io and maintains send/receive buffers. Fastcgipp::Request is designed to handle the individual requests themselves. The aspects of the FastCGI protocol itself are defined in the Fastcgipp::Protocol namespace.

The Fastcgipp::Request class is a pure virtual class. The class, as is, establishes and parses session data. Once complete it looks to user defined virtual functions for actually generating the response. A response shall be outputted by the user defined virtuals through an output stream. Once a request has control over operation it maintains it until relinquishing it. Should the user know a request will sit around waiting for data, it can return control to Fastcgipp::Manager and have a message sent back through the manager when the data is ready. The aspects of the session are build around the Fastcgipp::Http namespace.

Fastcgipp::Manager basically runs an endless loop (which can be terminated through POSIX signals or a function call from another thread) that passes control to requests that have a message queued or the transceiver. It is smart enough to go into a sleep mode when there are no tasks to complete or data to receive.

Fastcgipp::Transceiver's transmit half implements a cyclic buffer that can grow indefinitely to insure that operation does not halt. The send half receives full frames and passes them through Fastcgipp::Manager onto the requests. It manages all the open connections and polls them for incoming data.

\section dep Dependencies

	\li Boost C++ Libraries >1.35.0
	\li Posix compliant OS (socket stuff)

\section installation Installation

The installation of fastcgi++ is pretty standard save a few quirks. The most basic installation of the library is the traditional:

<tt>tar -xvjf fastcgi++-$(VERSION).tar.bz2\n
cd fastcgi++-$(VERSION)\n
make\n
make install</tt>

The default prefix for installation is /usr/local. If you wanted to change it to /usr simply change <tt>"make install"</tt> to <tt>"PREFIX=/usr make install"</tt>. If you want the binary files to be run through \c strip change \c "make" to <tt>"STRIP=true make"</tt>.

To also install the documentation into $PREFIX/share/doc/fastcgi++ with PREFIX preceded as above run this:

<tt>make doc-install</tt>

If you want to build and install the examples, simply the commands below. The build the examples statically, precede make with STATIC=yes.

<tt>make examples\n
make examples-install</tt>

The examples will by default install to $WWWROOT/fastcgipp with WWWROOT = /var/www/localhost/htdocs. To change WWWROOT, simply precede <tt>"make examples-install"</tt> with a definition of WWWROOT as in the prefix example above.

\section tutorials Tutorials

This is a collection of tutorials that should cover most aspects of the fastcgi++ library

\subpage helloWorld : A simple tutorial outputting "Hello World" in five languages using UTF-32 internally and UTF-8 externally.

\subpage echo : An example of a FastCGI application that echoes all user data and sets a cookie

\subpage showGnu : A tutorial explaining how to display images and non-html data as well as setting locales

\subpage timer : A tutorial covering the use of the task manager and threads to have requests efficiently communicate with non-fastcgi++ data.

*/

/*!

\page timer Delayed response

\section timerTutorial Tutorial

Our goal here will be to make a FastCGI application that responds to clients with some text, waits five seconds and then sends more. We're going to use threading and boost::asio to handle our timer. Your going to need the boost C++ libraries for this. At least version 1.35.0.

All code and data is located in the examples directory of the tarball. Make sure to link this with the following library options: -lfastcgipp -lboost_thread -lboost_system

\subsection timerError Error Logging

Our first step will be setting up an error logging system. Although requests can log errors directly to the HTTP server error log, I like to have an error logging system that's separate from the library when testing applications. Let's set up a function that takes a c style string and logs it to a file with a timestamp. Since everyone has access to the /tmp directory, I set it up to send error messages to /tmp/errlog. You can change it if you want to.

\code
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>

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
\endcode

\subsection timerRequest Request Handler

Now we need to write the code that actually handles the request. In this examples we still need to derive from Fastcgipp::Request and define the Fastcgipp::Request::response() function, but also some more. We're also going to need some member data to keep track of our requests execution state, a default constructor to initialize it and a global boost::asio::io_service object. In this example let's just use plain old ISO-8859-1 and pass char as the template parameter.

\code
#include <fastcgi++/request.hpp>

#include <cstring>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

boost::asio::io_service io;

class Timer: public Fastcgipp::Request<char>
{
private:
\endcode

We'll start with our data to keep track of the execution state and a pointer for our timer object.

\code
	enum State { START, FINISH } state;
	boost::scoped_ptr<boost::asio::deadline_timer> t;
\endcode

Now we can define our response function. It is this function that is called to generate a response for the client. We'll start it off with a switch statement that tests our execution state. It isn't a good idea to define the response() function inline as it is called from numerous spots, but for the examples readability we will make an exception.

\code
	bool response()
	{
		switch(state)
		{
			case START:
			{
\endcode

First thing we'll do is output our HTTP header. Note the charset=ISO-8859-1. Remember that HTTP headers must be terminated with "\r\n\r\n". NOT just "\n\n".

\code
				out << "Content-Type: text/html; charset=ISO-8859-1\r\n\r\n";
\endcode

Some standard HTML header output

\code
				out << "<html><head><meta http-equiv='Content-Type' content='text/html; charset=ISO-8859-1' />";
				out << "<title>fastcgi++: Threaded Timer</title></head><body>";
\endcode

Output a message saying we are starting the timer

\code
				out << "Starting Timer...<br />";
\endcode

If we want to the client to see everything that we just outputted now instead of when the request is complete, we will have to flush the stream buffer as it's just sitting in there now.

\code
				out.flush();
\endcode

Now let's make a five second timer.

\code
				t.reset(new boost::asio::deadline_timer(io, boost::posix_time::seconds(5)));
\endcode

Now we work with our Fastcgipp::Request::callback. It is a boost::function that takes a Fastcgipp::Message as a single argument. This callback function will pass the message on to this request thereby having Fastcgipp::Request::response() called function again. The callback function is thread safe. That means you can pass messages back to requests from other threads.

First we'll build the message we want sent back here. Normally the message would be built by whatever is calling the callback, but this is just a simple example. A type of 0 means a FastCGI record and is used internally. All other values we can use ourselves to define different message types (sql queries, file grabs, etc...). In this example we will use type=1 for timer stuff.

\code
				Fastcgipp::Message msg;
				msg.type=1;

				{
					char cString[] = "I was passed between two threads!!";
					msg.size=sizeof(cString);
					msg.data.reset(new char[sizeof(cString)]);
					std::strncpy(msg.data.get(), cString, sizeof(cString));
				}
\endcode

Now we can give our callback function to our timer.

\code
				t->async_wait(boost::bind(callback, msg));
\endcode

We need to set our state to FINISH so that when this response is called a second time, we don't repeat this.

\code
				state=FINISH;
\endcode

Now we will return and allow the task manager to do other things (or sleep if there is nothing to do). We must return false if the request is not yet complete.

\code
				return false;
			}
\endcode

Next step, we define what's done when Fastcgipp::Request::responce() is called a second time.

\code
			case FINISH:
			{
\endcode

Whenever Fastcgipp::Request::response() is called, the Fastcgipp::Message that lead to it's calling is stored in Fastcgipp::Request::message.

\code
				out << "Timer Finished! Our message data was \"" << message.data.get() << "\"";
				out << "</body></html>";
\endcode

And we're basically done defining our response! All we need to do is return a boolean value. Always return true if you are done. This will let apache and the manager know we are done so they can destroy the request and free it's resources.

\code
				return true;
			}
		}
	}
\endcode

Now we can define our default constructor.

\code
public:
	Timer(): state(START) {}
};
\endcode

\subsection timerManager Requests Manager

Now we need to make our main() function. In addition to creating a Fastcgipp::Manager object with the new class we made as a template parameter and calling it's handler, we also need to set up our threads and io stuff. This isn't a boost::asio tutorial, check the boost documentation for clarification on it.

\code
#include <fastcgi++/manager.hpp>
int main()
{
	try
	{
		boost::asio::io_service::work w(io);
		boost::thread t(boost::bind(&boost::asio::io_service::run, &io));
		Fastcgipp::Manager<Timer> fcgi;
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
\endcode

\section timerCode Full Source Code

\code
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cstring>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
boost::asio::io_service io;

#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>

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

class Timer: public Fastcgipp::Request<char>
{
private:
	enum State { START, FINISH } state;

	boost::scoped_ptr<boost::asio::deadline_timer> t;

	bool response()
	{
		switch(state)
		{
			case START:
			{
				out << "Content-Type: text/html; charset=ISO-8859-1\r\n\r\n";

				out << "<html><head><meta http-equiv='Content-Type' content='text/html; charset=ISO-8859-1' />";
				out << "<title>fastcgi++: Threaded Timer</title></head><body>";
				
				out << "Starting Timer...<br />";

				out.flush();

				t.reset(new boost::asio::deadline_timer(io, boost::posix_time::seconds(5)));

				Fastcgipp::Message msg;
				msg.type=1;

				{
					char cString[] = "I was passed between two threads!!";
					msg.size=sizeof(cString);
					msg.data.reset(new char[sizeof(cString)]);
					std::strncpy(msg.data.get(), cString, sizeof(cString));
				}

				t->async_wait(boost::bind(callback, msg));

				state=FINISH;

				return false;
			}
			case FINISH:
			{
				out << "Timer Finished! Our message data was \"" << message.data.get() << "\"";
				out << "</body></html>";

				return true;
			}
		}
	}
public:
	Timer(): state(START) {}
};

int main()
{
	try
	{
		boost::asio::io_service::work w(io);
		boost::thread t(boost::bind(&boost::asio::io_service::run, &io));

		Fastcgipp::Manager<Timer> fcgi;
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
\endcode

*/

/*!

\page showGnu Display The Gnu

\section showGnuTutorial Tutorial

Our goal here is simple and easy. All we want to do is show the gnu.png file and effectively utilize caching.

All code and data is located in the examples directory of the tarball. Make sure to link this with the following library options: -lfastcgipp -lboost_thread

\subsection showGnuError Error Logging

Our first step will be setting up an error logging system. Although requests can log errors directly to the HTTP server error log, I like to have an error logging system that's separate from the library when testing applications. Let's set up a function that takes a c style string and logs it to a file with a timestamp. Since everyone has access to the /tmp directory, I set it up to send error messages to /tmp/errlog. You can change it if you want to.

\code
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>

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
\endcode

\subsection showGnuRequest Request Handler

Now we need to write the code that actually handles the request. Quite simply, all we need to do is derive from Fastcgipp::Request and define the Fastcgipp::Request::response() function. Since we're just outputting an image, we don't need to bother with Unicode and can pass char as the template parameter.

\code
#include <fastcgi++/request.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class ShowGnu: public Fastcgipp::Request<char>
{
\endcode

Now we can define our response function. It is this function that is called to generate a response for the client. It isn't a good idea to define the response() function inline as it is called from numerous spots, but for the examples readability we will make an exception.

\code
	bool response()
	{
		using namespace std;
		using namespace boost;
\endcode

We are going to use boost::posix_time::ptime to communicate the images modification time for cache purposes.

\code
		posix_time::ptime modTime;
		int fileSize;
		int etag;
\endcode

We'll use the POSIX stat() function (man 2 stat) to get the modification time, file size and inode number.

\code
		{
			struct stat fileStat;
			stat("gnu.png", &fileStat);
			fileSize = fileStat.st_size;
			modTime = posix_time::from_time_t(fileStat.st_mtime);
\endcode

Fastcgipp::Http::Session implements the etag variable as an integer for better processing efficiency.

\code
			etag = fileStat.st_ino;
		}
\endcode

We will need to call Fastcgipp::Request::setloc() to set a facet in our requests locale regarding how to format the date upon insertion. It needs to conform to the HTTP standard. When setting locales for the streams, make sure to use the Fastcgipp::Request::setloc() function instead of directly imbueing them. This insures that the UTF-8 code conversion still functions properly if used.

\code
		setloc(locale(loc, new posix_time::time_facet("%a, %d %b %Y %H:%M:%S GMT")));
\endcode

If the modification time of the file is older or equal to the if-modified-since value sent to us from the client and the etag matches, we don't need to send the image to them.

\code
		if(!session.ifModifiedSince.is_not_a_date_time() && etag==session.etag && modTime<=session.ifModifiedSince)
		{
			out << "Status: 304 Not Modified\r\n\r\n";
			return true;
		}
\endcode

We're going to use std::fstream to read the file data.

\code
		ifstream image("gnu.png");
\endcode

Now we transmit our HTTP header containing the modification data, file size and etag value.

\code
		out << "Last-Modified: " << modTime << '\n';
		out << "Etag: " << etag << '\n';
		out << "Content-Length: " << fileSize << '\n';
		out << "Content-Type: image/png\r\n\r\n";
\endcode

Now that the header is sent, we can transmit the actual image. To send raw binary data to the client, the streams have a dump function that bypasses the stream buffer and it's code conversion. The function is overloaded to either Fastcgipp::Fcgistream::dump(std::basic_istream<char>& stream) or Fastcgipp::Fcgistream::dump(char* data, size_t size). Remember that if we are using wide characters internally, the stream converts anything sent into the stream to UTF-8 before transmitting to the client. If we want to send binary data, we definitely don't want any code conversion so that is why this function exists.

\code
		out.dump(image);
\endcode

And we're basically done defining our response! All we need to do is return a boolean value. Always return true if you are done. This will let apache and the manager know we are done so they can destroy the request and free it's resources. Return false if you are not finished but want to relinquish control and allow other requests to operate. You would do this if the request needed to wait for a message to be passed back to it through the task manager.

\code
		return true;
	}
};
\endcode

\subsection showGnuManager Requests Manager

Now we need to make our main() function. Really all one needs to do is create a Fastcgipp::Manager object with the new class we made as a template parameter, then call it's handler. Let's go one step further though and set up a try/catch loop in case we get any exceptions and log them with our error_log function.

\code
#include <fastcgi++/manager.hpp>
int main()
{
	try
	{
		Fastcgipp::Manager<ShowGnu> fcgi;
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
\endcode

\section showGnuCode Full Source Code

\code
#include <fstream>
#include "boost/date_time/posix_time/posix_time.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>

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

class ShowGnu: public Fastcgipp::Request<char>
{
	// Now we define the actual function that sends a response to the client.
	bool response()
	{
		using namespace std;
		using namespace boost;

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

		if(!session.ifModifiedSince.is_not_a_date_time() && etag==session.etag && modTime<=session.ifModifiedSince)
		{
			out << "Status: 304 Not Modified\r\n\r\n";
			return true;
		}

		std::ifstream image("gnu.png");

		out << "Last-Modified: " << modTime << '\n';
		out << "Etag: " << etag << '\n';
		out << "Content-Length: " << fileSize << '\n';
		out << "Content-Type: image/png\r\n\r\n";

		out.dump(image);
		return true;
	}
};

int main()
{
	try
	{
		Fastcgipp::Manager<ShowGnu> fcgi;
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}

\endcode

*/

/*!

\page echo Echo

\section echoTutorial Tutorial

Our goal here will be to make a FastCGI application that responds to clients with an echo of all session data that was processed. This will include HTTP header data along with post data that was transmitted by the client. Since we want to be able to echo any alphabets, our best solution is to use UTF-32 wide characters internally and have the library code convert it to UTF-8 before sending it to the client. Your going to need the boost C++ libraries for this. At least version 1.35.0.

All code and data is located in the examples directory of the tarball. Make sure to link this with the following library options: -lfastcgipp -lboost_thread

\subsection echoError Error Handling

Our first step will be setting up an error logging system. Although requests can log errors directly to the HTTP server error log, I like to have an error logging system that's separate from the library when testing applications. Let's set up a function that takes a c style string and logs it to a file with a timestamp. Since everyone has access to the /tmp directory, I set it up to send error messages to /tmp/errlog. You can change it if you want to.

\code
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>

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
\endcode

\subsection echoRequest Request Handler

Now we need to write the code that actually handles the request. Quite simply, all we need to do is derive from Fastcgipp::Request and define the Fastcgipp::Request::response() function. Since we've decided to use wide Unicode characters, we need to pass wchar_t as the template parameter to the Request class as opposed to char.

\code
#include <fastcgi++/request.hpp>

class HelloWorld: public Fastcgipp::Request<wchar_t>
{
\endcode

Now we can define our response function. It is this function that is called to generate a response for the client. It isn't a good idea to define the response() function inline as it is called from numerous spots, but for the examples readability we will make an exception.

\code
	bool response()
	{
\endcode

First thing we need to do is output our HTTP header. Let us start with settting a cookie in the client using a Unicode string. Just to see how it echoes.

\code
		wchar_t langString[] = { 0x0440, 0x0443, 0x0441, 0x0441, 0x043a, 0x0438, 0x0439, 0x0000 };
		out << "Set-Cookie: lang=" << langString << '\n';
\endcode

Next of course we need to output our content type part of the header. Note the charset=utf-8. Remember that HTTP headers must be terminated with "\r\n\r\n". Not just "\n\n".

\code
		out << "Content-Type: text/html; charset=utf-8\r\n\r\n";
\endcode

Next we'll get some initial HTML stuff out of the way

\code
		out << "<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
		out << "<title>fastcgi++: Echo in UTF-8</title></head><body>";
\endcode

Now we are ready to start outputting session data. We'll start with the non-post session data. This data is defined and initialized in the session object which is of type Fastcgipp::Http::Session.

\code
		out << "<h1>Session Parameters</h1>";
		out << "<p><b>Hostname:</b> " << session.host << "<br />";
		out << "<b>User Agent:</b> " << session.userAgent << "<br />";
		out << "<b>Accepted Content Types:</b> " << session.acceptContentTypes << "<br />";
		out << "<b>Accepted Languages:</b> " << session.acceptLanguages << "<br />";
		out << "<b>Accepted Characters Sets:</b> " << session.acceptCharsets << "<br />";
		out << "<b>Referer:</b> " << session.referer << "<br />";
		out << "<b>Content Type:</b> " << session.contentType << "<br />";
		out << "<b>Query String:</b> " << session.queryString << "<br />";
		out << "<b>Cookies:</b> " << session.cookies << "<br />";
		out << "<b>Root:</b> " << session.root << "<br />";
		out << "<b>Script Name:</b> " << session.scriptName << "<br />";
		out << "<b>Content Length:</b> " << session.contentLength << "<br />";
		out << "<b>Keep Alive Time:</b> " << session.keepAlive << "<br />";
		out << "<b>Server Address:</b> " << session.serverAddress << "<br />";
		out << "<b>Server Port:</b> " << session.serverPort << "<br />";
		out << "<b>Client Address:</b> " << session.remoteAddress << "<br />";
		out << "<b>Client Port:</b> " << session.remotePort << "<br />";
		out << "<b>If Modified Since:</b> " << session.ifModifiedSince << "</p>";
\endcode

Next, we will make a little loop to output the post data. The post data is stored in the associative container session.posts of type Fastcgipp::Http::Session::Posts linking field names to Fastcgipp::Http::Post objects.

\code
		out << "<h1>Post Data</h1>";
\endcode

If there isn't any POST data, we'll just say so

\code
		if(session.posts.size())
			for(Fastcgipp::Http::Session<wchar_t>::Posts::iterator it=session.posts.begin(); it!=session.posts.end(); ++it)
			{
				out << "<h2>" << it->first << "</h2>";

				if(it->second.type==Fastcgipp::Http::Post<wchar_t>::form)
				{
					out << "<p><b>Type:</b> form data<br />";
					out << "<b>Value:</b> " << it->second.value << "</p>";
				}
				
				else
				{
					out << "<p><b>Type:</b> file<br />";
\endcode

When the post type is a file, the filename is stored in Post::value

\code
					out << "<b>Filename:</b> " << it->second.value << "<br />";
					out << "<b>Size:</b> " << it->second.size << "<br />";
					out << "<b>Data:</b></p><pre>";
\endcode

We will use Fastcgipp::Fcgistream::dump to send the file raw data directly to the client

\code
					out.dump(it->second.data.get(), it->second.size);
					out << "</pre>";
				}
			}
		else
			out << "<p>No post data</p>";
\endcode

And we're basically done defining our response! All we need to do is return a boolean value. Always return true if you are done. This will let apache and the manager know we are done so they can destroy the request and free it's resources. Return false if you are not finished but want to relinquish control and allow other requests to operate. You would do this if the request needed to wait for a message to be passed back to it through the task manager.

\code
		return true;
	}
};
\endcode

\subsection echoManager Requests Manager

Now we need to make our main() function. Really all one needs to do is create a Fastcgipp::Manager object with the new class we made as a template parameter, then call it's handler. Let's go one step further though and set up a try/catch loop in case we get any exceptions and log them with our error_log function.

\code
#include <fastcgi++/manager.hpp>

int main()
{
	try
	{
		Fastcgipp::Manager<Echo> fcgi;
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
\endcode

\section echoWorldCode Full Source Code

\code

#include <fstream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>

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

class Echo: public Fastcgipp::Request<wchar_t>
{
	bool response()
	{
		wchar_t langString[] = { 0x0440, 0x0443, 0x0441, 0x0441, 0x043a, 0x0438, 0x0439, 0x0000 };
		out << "Set-Cookie: lang=" << langString << '\n';

		out << "Content-Type: text/html; charset=utf-8\r\n\r\n";

		out << "<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
		out << "<title>fastcgi++: Echo in UTF-8</title></head><body>";

		out << "<h1>Session Parameters</h1>";
		out << "<p><b>Hostname:</b> " << session.host << "<br />";
		out << "<b>User Agent:</b> " << session.userAgent << "<br />";
		out << "<b>Accepted Content Types:</b> " << session.acceptContentTypes << "<br />";
		out << "<b>Accepted Languages:</b> " << session.acceptLanguages << "<br />";
		out << "<b>Accepted Characters Sets:</b> " << session.acceptCharsets << "<br />";
		out << "<b>Referer:</b> " << session.referer << "<br />";
		out << "<b>Content Type:</b> " << session.contentType << "<br />";
		out << "<b>Query String:</b> " << session.queryString << "<br />";
		out << "<b>Cookies:</b> " << session.cookies << "<br />";
		out << "<b>Root:</b> " << session.root << "<br />";
		out << "<b>Script Name:</b> " << session.scriptName << "<br />";
		out << "<b>Content Length:</b> " << session.contentLength << "<br />";
		out << "<b>Keep Alive Time:</b> " << session.keepAlive << "<br />";
		out << "<b>Server Address:</b> " << session.serverAddress << "<br />";
		out << "<b>Server Port:</b> " << session.serverPort << "<br />";
		out << "<b>Client Address:</b> " << session.remoteAddress << "<br />";
		out << "<b>Client Port:</b> " << session.remotePort << "<br />";
		out << "<b>If Modified Since:</b> " << session.ifModifiedSince << "</p>";

		out << "<h1>Post Data</h1>";
		if(session.posts.size())
			for(Fastcgipp::Http::Session<wchar_t>::Posts::iterator it=session.posts.begin(); it!=session.posts.end(); ++it)
			{
				out << "<h2>" << it->first << "</h2>";
				if(it->second.type==Fastcgipp::Http::Post<wchar_t>::form)
				{
					out << "<p><b>Type:</b> form data<br />";
					out << "<b>Value:</b> " << it->second.value << "</p>";
				}
				
				else
				{
					out << "<p><b>Type:</b> file<br />";
					out << "<b>Filename:</b> " << it->second.value << "<br />";
					out << "<b>Size:</b> " << it->second.size << "<br />";
					out << "<b>Data:</b></p><pre>";
					out.dump(it->second.data.get(), it->second.size);
					out << "</pre>";
				}
			}
		else
			out << "<p>No post data</p>";

		out << "</body></html>";
		return true;
	}
};

int main()
{
	try
	{
		Fastcgipp::Manager<Echo> fcgi;
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}

\endcode


*/

/*!

\page helloWorld Hello World in Five Languages

\section helloWorldTutorial Tutorial

Our goal here will be to make a FastCGI application that responds to clients with a "Hello World" in five different languages. Your going to need the boost C++ libraries for this. At least version 1.35.0. In this example we are going to use boost::asio to handle our timers and callback. Unfortunately because mod_fastcgi buffers the output before sending it to the client by default, we will only get to see the true effects of the timer if you put the following directive in your apache configuration: FastCgiConfig -flush

All code and data is located in the examples directory of the tarball. Make sure to link this with the following library options: -lfastcgipp -lboost_thread

\subsection helloWorldError Error Logging

Our first step will be setting up an error logging system. Although requests can log errors directly to the HTTP server error log, I like to have an error logging system that's separate from the library when testing applications. Let's set up a function that takes a c style string and logs it to a file with a timestamp. Since everyone has access to the /tmp directory, I set it up to send error messages to /tmp/errlog. You can change it if you want to.

\code
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>

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
\endcode

\subsection helloWorldRequest Request Handler

Now we need to write the code that actually handles the request. Quite simply, all we need to do is derive from Fastcgipp::Request and define the Fastcgipp::Request::response() function. Since we've decided to use wide Unicode characters, we need to pass wchar_t as the template parameter to the Request class as opposed to char.

\code
#include <fastcgi++/request.hpp>

class HelloWorld: public Fastcgipp::Request<wchar_t>
{
\endcode

Now we can define our response function. It is this function that is called to generate a response for the client. It isn't a good idea to define the response() function inline as it is called from numerous spots, but for the examples readability we will make an exception.

\code
	bool response()
	{
\endcode

Let's define our hello world character strings we are going to send to the client. Unfortunately C++ doesn't yet support Unicode string literals, but it is just around the corner. Obviously we could have read this data in from a UTF-8 file, but in this example I found it easier to just use these arrays.

\code
		wchar_t russian[]={ 0x041f, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442, 0x0020, 0x043c, 0x0438, 0x0440, 0x0000 };
		wchar_t chinese[]={ 0x4e16, 0x754c, 0x60a8, 0x597d, 0x0000 };
		wchar_t greek[]={ 0x0393, 0x03b5, 0x03b9, 0x03b1, 0x0020, 0x03c3, 0x03b1, 0x03c2, 0x0020, 0x03ba, 0x03cc, 0x03c3, 0x03bc, 0x03bf, 0x0000 };
		wchar_t japanese[]={ 0x4eca, 0x65e5, 0x306f, 0x4e16, 0x754c, 0x0000 };
		wchar_t runic[]={ 0x16ba, 0x16d6, 0x16da, 0x16df, 0x0020, 0x16b9, 0x16df, 0x16c9, 0x16da, 0x16de, 0x0000 };
\endcode

Any data we want to send to the client just get's inserted into the requests Fastcgipp::Fcgistream "out" stream. It works just the same as cout in almost every way. We'll start by outputting an HTTP header to the client. Note the "charset=utf-8" and keep in mind that proper HTTP headers are to be terminated with "\r\n\r\n"; not just "\n\n".

\code
		out << "Content-Type: text/html; charset=utf-8\r\n\r\n";
\endcode

Now we're ready to insert all the HTML data into the stream.

\code
		out << "<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
		out << "<title>fastcgi++: Hello World in UTF-8</title></head><body>";
		out << "English: Hello World<br />";
		out << "Russian: " << russian << "<br />";
		out << "Greek: " << greek << "<br />";
		out << "Chinese: " << chinese << "<br />";
		out << "Japanese: " << japanese << "<br />";
		out << "Runic English?: " << runic << "<br />";
		out << "</body></html>";
\endcode

We'll also output a little hello to the HTTP server error log just for fun as well.

\code
		err << "Hello apache error log";
\endcode

And we're basically done defining our response! All we need to do is return a boolean value. Always return true if you are done. This will let apache and the manager know we are done so they can destroy the request and free it's resources. Return false if you are not finished but want to relinquish control and allow other requests to operate. You would do this if the request needed to wait for a message to be passed back to it through the task manager.

\code
		return true;
	}
};
\endcode

\subsection helloWorldManager Requests Manager

Now we need to make our main() function. Really all one needs to do is create a Fastcgipp::Manager object with the new class we made as a template parameter, then call it's handler. Let's go one step further though and set up a try/catch loop in case we get any exceptions and log them with our error_log function.

\code
#include <fastcgi++/manager.hpp>
int main()
{
	try
	{
		Fastcgipp::Manager<HelloWorld> fcgi;
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
\endcode

And that's it! About as simple as it gets.

\section helloWorldCode Full Source Code

\code
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>

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

class HelloWorld: public Fastcgipp::Request<wchar_t>
{
	bool response()
	{
		wchar_t russian[]={ 0x041f, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442, 0x0020, 0x043c, 0x0438, 0x0440, 0x0000 };
		wchar_t chinese[]={ 0x4e16, 0x754c, 0x60a8, 0x597d, 0x0000 };
		wchar_t greek[]={ 0x0393, 0x03b5, 0x03b9, 0x03b1, 0x0020, 0x03c3, 0x03b1, 0x03c2, 0x0020, 0x03ba, 0x03cc, 0x03c3, 0x03bc, 0x03bf, 0x0000 };
		wchar_t japanese[]={ 0x4eca, 0x65e5, 0x306f, 0x4e16, 0x754c, 0x0000 };
		wchar_t runic[]={ 0x16ba, 0x16d6, 0x16da, 0x16df, 0x0020, 0x16b9, 0x16df, 0x16c9, 0x16da, 0x16de, 0x0000 };

		out << "Content-Type: text/html; charset=utf-8\r\n\r\n";

		out << "<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
		out << "<title>fastcgi++: Hello World in UTF-8</title></head><body>";
		out << "English: Hello World<br />";
		out << "Russian: " << russian << "<br />";
		out << "Greek: " << greek << "<br />";
		out << "Chinese: " << chinese << "<br />";
		out << "Japanese: " << japanese << "<br />";
		out << "Runic English?: " << runic << "<br />";
		out << "</body></html>";

		err << "Hello apache error log";

		return true;
	}
};

int main()
{
	try
	{
		Fastcgipp::Manager<HelloWorld> fcgi;
		fcgi.handler();
	}
	catch(std::exception& e)
	{
		error_log(e.what());
	}
}
\endcode

*/
