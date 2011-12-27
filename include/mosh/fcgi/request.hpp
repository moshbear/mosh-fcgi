//! \file request.hpp Defines the Fastcgippm0sh::Request class
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


#ifndef MOSH_FCGI_REQUEST_HPP
#define MOSH_FCGI_REQUEST_HPP

#include <locale>
#include <queue>
#include <map>
#include <string>
#include <mutex>
#include <functional>

#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/protocol/end_request.hpp>
#include <mosh/fcgi/protocol/message.hpp>
#include <mosh/fcgi/exceptions.hpp>
#include <mosh/fcgi/bits/block.hpp>
#include <mosh/fcgi/bits/iconv.hpp>
#include <mosh/fcgi/bits/iconv_cvt.hpp>
#include <mosh/fcgi/transceiver.hpp>
#include <mosh/fcgi/fcgistream.hpp>
#include <mosh/fcgi/http.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

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
 * @tparam char_type Character type for internal processing (wchar_t or char)
 * @tparam post_vec_type Vector type for storing file data
 * @tparam data_vec_type Vector type for storing fcgi_data 
 */
template <class char_type, typename post_val_type = std::basic_string<char_type>, typename data_buf_type = std::vector<char>>
class Request {
public:
	//! Initializes what it can. set() must be called by Manager before the data is usable.
	Request()
		: state(protocol::Record_type::params) {
		setloc(std::locale::classic());
		out.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	}

protected:
	//! Structure containing all HTTP session data
	http::Fcgi_session<char_type, post_val_type, data_buf_type> session;

	// To dump data into the stream without it being code converted and bypassing the stream buffer
	// call Fcgistream::dump(char* data, size_t size)
	// or Fcgistream::dump(std::basicistream<char>& stream)

	//! Standard output stream to the client
	/*!
	 * To dump data directly through the stream without it being code converted and bypassing
	 * the stream buffer call Fcgistream::dump()
	 */
	Fcgistream<char_type, std::char_traits<char_type>> out;

	//! Output stream to the HTTP server error log
	/*!
	 * To dump data directly through the stream without it being code converted and bypassing
	 * the stream buffer call Fcgistream::dump()
	 */
	Fcgistream<char_type, std::char_traits<char_type>> err;

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
	 * @param[in] bytes_received Amount of bytes received in this FastCGI record
	 */
	virtual bool in_handler(int bytes_received) { return true; }

	//! Generate a filter data input response
	/*!
	 * This function exists should the library user wish to do something like generate a partial response based on
	 * bytes received from the client. The function is called by handler() every time a FastCGI DATA record is received.
	 * The function has no access to the data, but knows exactly how much was received based on the value that was passed.
	 * Note this value represents the amount of data received in the individual record, not the total amount received in
	 * the session. If the library user wishes to have such a value they would have to keep a tally of all size values
	 * passed.
	 *
	 * @param[in] bytes_received Amount of bytes received in this FastCGI record
	 */
	virtual bool data_handler(int bytes_received) { return true; }
	
	//! The locale associated with the request. Should be set with setloc(), not directly.
	std::locale loc;

	//! The message associated with the current handler() call.
	/*!
	 * This is only of use to the library user when a non FastCGI (type=0) Message is passed
	 * by using the requests callback.
	 *
	 * @sa callback
	 */
	protocol::Message message;

	//! Set the requests locale
	/*!
	 * This function both sets loc to the locale passed to it and imbues the locale into the
	 * out and err stream. The user should always call this function as opposed to setting the
	 * locales directly is this functions insures the iconv code conversion is functioning properly.
	 *
	 * @param[in] loc New locale
	 * @sa loc
	 * @sa out
	 */
	void setloc(std::locale loc) {
		loc = std::locale(loc, new Iconv_cvt<char_type, char>);
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
	std::function<void(protocol::Message)> callback;
private:
	//@{
	//! A queue of messages to be handled by the request
	std::queue<protocol::Message> messages;
	//! Mutex for messages
	std::mutex messages_lock;
	//@}

	//! Request Handler
	/*!
	 * This function is called by Manager::handler() to handle messages destined for the request.
	 * It deals with FastCGI messages (type=0) while passing all other messages off to response().
	 *
	 * @return Boolean value indicating completion (true means complete)
	 * @sa callback
	 */
	bool handler() {
		using namespace protocol;
		using namespace std;

		try {
			{
				std::lock_guard<std::mutex> lock(messages_lock);
				message = messages.front();
				messages.pop();
			}
			if (!message.type) {
				aligned<sizeof(Header), Header> _header(static_cast<const void*>(message.data.get()));
				Header& header = _header;
				const char* body = message.data.get() + sizeof(Header);
				switch (header.type()) {
				case Record_type::params: {
					if (state != Record_type::params)
						throw exceptions::Record_out_of_order(id, state, Record_type::params);
					if (header.content_length() == 0) {
						if (role == Role::authorizer) {
							state = Record_type::out;
							if (response()) {
								complete(0);
								return true;
							}
							break;
						}
						state = Record_type::in;
						break;
					}
					session.fill(body, header.content_length());
				} break;
				case Record_type::in: {
					if (state != Record_type::in)
						throw exceptions::Record_out_of_order(id, state, Record_type::in);
					if (header.content_length() == 0) {
						if (role == Role::filter) {
							state = Record_type::data;
							break;
						}
						state = Record_type::out;
						if (response()) {
							complete(0);
							return true;
						}
						break;
					}
					session.fill_post(body, header.content_length());
					in_handler(header.content_length());
				} break;
				case Record_type::data: {
					if (state != Record_type::data)
						throw exceptions::Record_out_of_order(id, state, Record_type::data);
					if (header.content_length() == 0) {
						state = Record_type::out;
						if (response()) {
							complete(0);
							return true;
						}
					}
					session.fill_data(body, header.content_length());
					data_handler(header.content_length());
				} break;
				case Record_type::abort_request:
					return true;
				default:;
				}
			} else if (response()) {
				complete(0);
				return true;
			}
		} catch (std::exception& e) {
			err << e.what() << std::endl;
			complete(1);
			return true;
		}
		return false;
	}

	template <typename T> friend class Manager;
	//! Pointer to the transceiver object that will send data to the other side
	Transceiver* transceiver;
	//! The role that the other side expects this request to play
	protocol::Role role;
	//! The complete ID (request id & file descriptor) associated with the request
	protocol::Full_id id;
	//! Boolean value indicating whether or not the file descriptor should be closed upon completion.
	bool kill_con;
	//! What the request is current doing
	protocol::Record_type state;
	//! Generates an ENDREQUEST FastCGI record
	void complete(int app_status) {
		using namespace protocol;
		out.flush();
		err.flush();

		Header hdr(version, Record_type::end_request, id.fcgi_id, sizeof(End_request), 0);
		End_request ereq(app_status, Protocol_status::request_complete);

		Block buffer(transceiver->request_write(sizeof(Header) + sizeof(End_request)));

		memcpy(buffer.data, &hdr, sizeof(Header));
		memcpy(buffer.data + sizeof(Header), &ereq, sizeof(End_request));

		transceiver->secure_write(sizeof(Header) + sizeof(End_request), id, kill_con);
	}

	/*! @brief Set's up the request with the data it needs.
	 * This function is an "after-the-fact" constructor that build vital initial data for the request.
	 *
	 * @param[in] id Complete ID of the request
	 * @param[in] transceiver Transceiver object the request will use
	 * @param[in] role The role that the other side expects this request to play
	 * @param[in] kill_con Boolean value indicating whether or not the file descriptor should be closed upon completion
	 * @param[in] callback Callback function capable of passing messages to the request
	 */
	void set(protocol::Full_id id, Transceiver& transceiver, protocol::Role role, bool kill_con,
			std::function<void(protocol::Message)> callback)
	{
		this->kill_con = kill_con;
		this->id = id;
		this->transceiver = &transceiver;
		this->role = role;
		this->callback = callback;

		err.set(id, transceiver, protocol::Record_type::err);
		out.set(id, transceiver, protocol::Record_type::out);
	}
};

MOSH_FCGI_END


#endif
