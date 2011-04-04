//! \file request.hpp Defines the Fastcgipp_m0sh::Request class
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


#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <queue>
#include <map>
#include <string>

#include <boost/shared_array.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include <fastcgipp-mosh/protocol.hpp>
#include <fastcgipp-mosh/exceptions.hpp>
#include <fastcgipp-mosh/transceiver.hpp>
#include <fastcgipp-mosh/fcgistream.hpp>
#include <fastcgipp-mosh/bits/utf8_codecvt.hpp>
#include <fastcgipp-mosh/http.hpp>

//! Topmost namespace for the fastcgi++ library
namespace Fastcgipp_m0sh
{


//! %Request handling class
/*!
 * Derivations of this class will handle requests. This
 * includes building the session data, processing post/get data,
 * fetching data (files, database), and producing a response.
 * Once all client data is organized, response() will be called.
 * At minimum, derivations of this class must define response().
 *
 * If you want to use UTF-8 encoding pass wchar_t as the template
 * argument, use setloc() to setup a UTF-8 locale and use wide
 * character unicode internally for everything. If you want to use
 * a 8bit character set encoding pass char as the template argument and
 * setloc() a locale with the corresponding character set.
 *
 * @tparam charT Character type for internal processing (wchar_t or char)
 */
template <class charT,
	  typename postvecT = std::vector<char>,
	  typename datavecT = postvecT>
class Request
{
public:
	//! Initializes what it can. set() must be called by Manager before the data is usable.
	Request()
		: state(Protocol::PARAMS) {
		setloc(std::locale::classic());
		out.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	}

protected:
	//! Structure containing all HTTP session data
	Http::Environment<charT, postvecT, datavecT> session;

	// To dump data into the stream without it being code converted and bypassing the stream buffer
	// call Fcgistream::dump(char* data, size_t size)
	// or Fcgistream::dump(std::basic_istream<char>& stream)

	//! Standard output stream to the client
	/*!
	 * To dump data directly through the stream without it being code converted and bypassing
	 * the stream buffer call Fcgistream::dump()
	 */
	Fcgistream<charT, std::char_traits<charT> > out;

	//! Output stream to the HTTP server error log
	/*!
	 * To dump data directly through the stream without it being code converted and bypassing
	 * the stream buffer call Fcgistream::dump()
	 */
	Fcgistream<charT, std::char_traits<charT> > err;

	//! Response generator
	/*!
	 * This function is called by handler() once all request data has been received from the other side or if a
	 * Message not of a FastCGI type has been passed to it. The function shall return true if it has completed
	 * the response and false if it has not (waiting for a callback message to be sent).
	 *
	 * @return Boolean value indication completion (true means complete)
	 * @sa callback
	 */
	virtual bool response() = 0;

	//! Generate a data input response
	/*!
	 * This function exists should the library user wish to do something like generate a partial response based on
	 * bytes received from the client. The function is called by handler() every time a FastCGI IN record is received.
	 * The function has no access to the data, but knows exactly how much was received based on the value that was passed.
	 * Note this value represents the amount of data received in the individual record, not the total amount received in
	 * the session. If the library user wishes to have such a value they would have to keep a tally of all size values
	 * passed.
	 *
	 * @param[in] bytesReceived Amount of bytes received in this FastCGI record
	 */
	virtual bool inHandler(int bytesReceived) { return true; }

	//! Generate a filter data input response
	/*!
	 * This function exists should the library user wish to do something like generate a partial response based on
	 * bytes received from the client. The function is called by handler() every time a FastCGI DATA record is received.
	 * The function has no access to the data, but knows exactly how much was received based on the value that was passed.
	 * Note this value represents the amount of data received in the individual record, not the total amount received in
	 * the session. If the library user wishes to have such a value they would have to keep a tally of all size values
	 * passed.
	 *
	 * @param[in] bytesReceived Amount of bytes received in this FastCGI record
	 */
	virtual bool dataHandler(int bytesReceived) { return true; }
	//! The locale associated with the request. Should be set with setloc(), not directly.
	std::locale loc;

	//! The message associated with the current handler() call.
	/*!
	 * This is only of use to the library user when a non FastCGI (type=0) Message is passed
	 * by using the requests callback.
	 *
	 * @sa callback
	 */
	Message message;

	//! Set the requests locale
	/*!
	 * This function both sets loc to the locale passed to it and imbues the locale into the
	 * out and err stream. The user should always call this function as opposed to setting the
	 * locales directly is this functions insures the utf8 code conversion is functioning properly.
	 *
	 * @param[in] loc_ New locale
	 * @sa loc
	 * @sa out
	 */
	void setloc(std::locale loc_) {
		loc = makeLocale<charT>(loc_);
		out.imbue(loc);
		err.imbue(loc);
	}

	//! Callback function for dealings outside the fastcgi++ library
	/*!
	 * The purpose of the callback object is to provide a thread safe mechanism for functions and
	 * classes outside the fastcgi++ library to talk to the requests. Should the library
	 * wish to have another thread process or fetch some data, that thread can call this
	 * function when it is finished. It is equivalent to this:
	 *
	 * void callback(Message msg);
	 *
	 *	The sole parameter is a Message that contains both a type value for processing by response()
	 *	and the raw castable data.
	 */
	boost::function<void(Message)> callback;
private:
	//! Queue type for pending messages
	/*!
	 * This is merely a derivation of a std::queue<Message> and a
	 * boost::mutex that gives data locking abilities to the STL container.
	 */
	class Messages: public std::queue<Message>, public boost::mutex {};
	//! A queue of messages to be handler by the request
	Messages messages;

	//! Request Handler
	/*!
	 * This function is called by Manager::handler() to handle messages destined for the request.
	 * It deals with FastCGI messages (type=0) while passing all other messages off to response().
	 *
	 * @return Boolean value indicating completion (true means complete)
	 * @sa callback
	 */
	bool handler() {
		using namespace Protocol;
		using namespace std;

		try {
			{
				boost::lock_guard<boost::mutex> lock(messages);
				message = messages.front();
				messages.pop();
			}
			if (!message.type) {
				const Header& header = *reinterpret_cast<Header*>(message.data.get());
				const char* body = message.data.get() + sizeof(Header);
				switch (header.getType()) {
				case PARAMS: {
					if (state != PARAMS)
						throw Exceptions::RecordOutOfOrder(id, state, PARAMS);
					if (header.getContentLength() == 0) {
						if (role == AUTHORIZER) {
							state = OUT;
							if (response()) {
								complete();
								return true;
							}
							break;
						}
						state = IN;
						break;
					}
					if (!session.fill(body, header.getContentLength()))
						throw Exceptions::Param(id);
				} break;
				case IN: {
					if (state != IN)
						throw Exceptions::RecordOutOfOrder(id, state, IN);
					if (header.getContentLength() == 0) {
						if (role == FILTER) {
							state = DATA;
							break;
						}
						state = OUT;
						if (response()) {
							complete();
							return true;
						}
						break;
					}
					session.fillPost(body, header.getContentLength());
					inHandler(header.getContentLength());
				} break;
				case DATA: {
					if (state != DATA)
						throw Exceptions::RecordOutOfOrder(id, state, DATA);
					if (header.getContentLength() == 0) {
						state = OUT;
						if (response()) {
							complete();
							return true;
						}
					}
					session.fillData(body, header.getContentLength());
					dataHandler(header.getContentLength());
				} break;
				case ABORT_REQUEST:
					return true;
				default:;
				}
			} else if (response()) {
				complete();
				return true;
			}
		} catch (std::exception& e) {
			err << e.what();
			err.flush();
			complete();
			return true;
		}
		return false;
	}

	template <typename T> friend class Manager;
	//! Pointer to the transceiver object that will send data to the other side
	Transceiver* transceiver;
	//! The role that the other side expects this request to play
	Protocol::Role role;
	//! The complete ID (request id & file descriptor) associated with the request
	Protocol::FullId id;
	//! Boolean value indicating whether or not the file descriptor should be closed upon completion.
	bool killCon;
	//! What the request is current doing
	Protocol::RecordType state;
	//! Generates an END_REQUEST FastCGI record
	void complete() {
		using namespace Protocol;
		out.flush();
		err.flush();

		Block buffer(transceiver->requestWrite(sizeof(Header) + sizeof(EndRequest)));

		Header& header = *reinterpret_cast<Header*>(buffer.data);
		header.setVersion(version);
		header.setType(END_REQUEST);
		header.setRequestId(id.fcgiId);
		header.setContentLength(sizeof(EndRequest));
		header.setPaddingLength(0);

		EndRequest& body = *reinterpret_cast<EndRequest*>(buffer.data + sizeof(Header));
		body.setAppStatus(0);
		body.setProtocolStatus(REQUEST_COMPLETE);

		transceiver->secureWrite(sizeof(Header) + sizeof(EndRequest), id, killCon);
	}

	//! Set's up the request with the data it needs.
	/*!
	 * This function is an "after-the-fact" constructor that build vital initial data for the request.
	 *
	 * @param[in] id_ Complete ID of the request
	 * @param[in] transceiver_ Transceiver object the request will use
	 * @param[in] role_ The role that the other side expects this request to play
	 * @param[in] killCon_ Boolean value indicating whether or not the file descriptor should be closed upon completion
	 * @param[in] callback_ Callback function capable of passing messages to the request
	 */
	void set(Protocol::FullId id_, Transceiver& transceiver_, Protocol::Role role_,
	         bool killCon_, boost::function<void(Message)> callback_) {
		killCon = killCon_;
		id = id_;
		transceiver = &transceiver_;
		role = role_;
		callback = callback_;

		err.set(id_, transceiver_, Protocol::ERR);
		out.set(id_, transceiver_, Protocol::OUT);
	}
};
}

#endif
