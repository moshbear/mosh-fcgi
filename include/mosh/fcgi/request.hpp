//! @file  mosh/fcgi/request.hpp Defines the MOSH_FCGI::Request class
/***************************************************************************
* Copyright (C) 2011-2 m0shbear                                            *
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

#ifndef MOSH_FCGI_REQUEST_HPP
#define MOSH_FCGI_REQUEST_HPP

#include <queue>
#include <map>
#include <string>
#include <mutex>
#include <functional>
#include <vector>

#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/protocol/message.hpp>
#include <mosh/fcgi/transceiver.hpp>
#include <mosh/fcgi/fcgistream.hpp>
#include <mosh/fcgi/http/session.hpp>
#include <mosh/fcgi/bits/locked.hpp>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @brief %Request handling class for POST-less requests 
 *
 * Derivations of this class will handle requests, namely interpreting
 * parameter K-Vs, IN records (POSTDATA), and FCGI_DATA (FILTERDATA)
 * records; processing the interpreted data; and producing a response.
 *
 * If you're writing an Authorizer, this is the Request that you want to derive.
 *
 * The following virtual methods are defined:
 * * params_handler() - When parameter parsing is complete, this function is
 * 	called to push the parameter list to the handler
 * * in_handler() - When an IN record is received, this function is called
 * 	to interpret the POSTDATA
 * * data_handler() - When a DATA record is received, this function is called
 * 	to interpret the FILTERDATA
 * * [pure] response() - When all the client data is received _or_ a non-FCGI message
 * 	is received, this function is called to process and produce a response
 *	or process the received message
 *
 */
class Request_base {
public:
	//! Initializes what it can. set() must be called by Manager before the data is usable.
	Request_base();
	virtual ~Request_base();

protected:
	/*! @brief Standard output stream to the client
	 *
	 * * To print UTF-8-encoded Unicode data, use operator&lt;&lt; with a @c wchar_t string
	 * * To print a buffered byte stream, use operator&lt&lt; with a @c char or @c uchar string
	 * * To dump data directly through the stream without buffering, call Fcgistream::dump().
	 */
	Fcgistream out;

	/*! @brief Output stream to the HTTP server error log
	 *
	 * * To print UTF-8-encoded Unicode data, use operator&lt;&lt; with a @c wchar_t string
	 * * To print a buffered byte stream, use operator&lt&lt; with a @c char or @c uchar string
	 * * To dump data directly through the stream without buffering, call Fcgistream::dump().
	 */
	Fcgistream err;

	/*! @brief Response generator
	 *
	 * This function is called by handler() once all request data has been received from the other side or if a
	 * Message not of a FastCGI type has been passed to it. The function shall return true if it has completed
	 * the response and false if it has not (waiting for a callback message to be sent).
	 *
	 * @return Boolean value indication completion (true means complete)
	 * @sa callback
	 */
	virtual bool response() = 0;
	//! Handler for parsed PARAMS
	virtual bool params_handler(std::pair<std::string, std::string> const& param) { return true; }
	//! Handler for POSTDATA
	virtual void in_handler(const uchar* data, size_t len) { }
	//! Handler for FCGI_DATA
	virtual void data_handler(const uchar* data, size_t len) { }
	
	/*! @brief The message associated with the current handler() call.
	 *
	 * This is only of use to the library user when a non FastCGI (type=0) Message is passed
	 * by using the requests callback.
	 *
	 * @sa callback
	 */
	protocol::Message message;

	/*! @brief Callback function for dealings outside the mosh-fcgi library
	 *
	 * The purpose of the callback object is to provide a thread safe mechanism for functions and
	 * classes outside the mosh-fcgi library to talk to the requests. Should the library
	 * wish to have another thread process or fetch some data, that thread can call this
	 * function when it is finished. It is equivalent to this:
	 *
	 * void callback(Message msg);
	 *
	 *	The sole parameter is a Message that contains both a type value for processing by response()
	 *	and the raw castable data.
	 */
	std::function<void(protocol::Message)> callback;
	//! Request parameters
	std::map<std::string, std::string> envs;

	//! Dump FastCGI request parameters to string
	/*! @note Does not dump message or envs
	 */
	u_string dump() const;
private:
	friend class Manager;
	//! A queue of messages to be handled by the request
	Mutexed<std::queue<protocol::Message>> messages;
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
	//! Param buffer
	u_string pbuf;

	/*! @brief Request Handler
	 *
	 * This function is called by Manager::handler() to handle messages destined for the request.
	 * It deals with FastCGI messages (type=0) while passing all other messages off to response().
	 *
	 * @return Boolean value indicating completion (true means complete)
	 * @sa callback
	 */
	bool handler();
	
	//! Generates an END_REQUEST FastCGI record
	void complete(int app_status);
	
	/*! @brief Set's up the request with the data it needs.
	 *
	 * This function is an "after-the-fact" constructor that build vital initial data for the request.
	 *
	 * @param[in] id Complete ID of the request
	 * @param[in] transceiver Transceiver object the request will use
	 * @param[in] role The role that the other side expects this request to play
	 * @param[in] kill_con Boolean value indicating whether or not the file descriptor should be closed upon completion
	 * @param[in] callback Callback function capable of passing messages to the request
	 */
	void set(protocol::Full_id id, Transceiver& transceiver, protocol::Role role, bool kill_con,
			std::function<void(protocol::Message)> callback);

	/*! @brief Fill params
	 *  @param term Terminal mode (send blank kv to param_handler to signal end of args list)
	 */
	void fill_params(bool term);
};



/*! @brief %Request handling with rudimentary buffering
 *
 * Request_base with buffered IN and DATA.
 *
 * @tparam Post_buf Vector type for storing IN stream
 * @tparam Data_buf Vector type for storing DATA stream (when role == Role::filter)
 *
 * @see Request_base
 */
template <typename Post_buf = std::vector<uchar>, typename Data_buf = std::vector<uchar>>
class Request : public virtual Request_base {
protected:
	//! Handler for IN records
	virtual void in_handler(const uchar* data, size_t len) {
		if (len > 0) {
			post_buf.reserve(post_buf.size() + len);
			std::copy(data, data + len, std::back_inserter(post_buf));
		}
		this->in_handler(len);
	}
	//! Handler for DATA records
	virtual void data_handler(const uchar* data, size_t len) {
		if (len > 0) {
			data_buf.reserve(data_buf.size() + len);
			std::copy(data, data + len, std::back_inserter(data_buf));
		}
		this->data_handler(len);
	}
	
	/*! @brief Generate a data input response
	 *
	 * This function exists should the library user wish to do something like generate a partial response based on
	 * bytes received from the client. The function is called by handler() every time a FastCGI IN record is received.
	 * The function has no access to the data, but knows exactly how much was received based on the value that was passed.
	 * Note this value represents the amount of data received in the individual record, not the total amount received in
	 * the session. If the library user wishes to have such a value they would have to keep a tally of all size values
	 * passed.
	 *
	 * @param[in] bytes_received Amount of bytes received in this FastCGI record
	 */
	virtual void in_handler(size_t bytes_received) { }

	/*! @brief Generate a filter data input response
	 *
	 * This function exists should the library user wish to do something like generate a partial response based on
	 * bytes received from the client. The function is called by handler() every time a FastCGI DATA record is received.
	 * The function has no access to the data, but knows exactly how much was received based on the value that was passed.
	 * Note this value represents the amount of data received in the individual record, not the total amount received in
	 * the session. If the library user wishes to have such a value they would have to keep a tally of all size values
	 * passed.
	 *
	 * @param[in] bytes_received Amount of bytes received in this FastCGI record
	 */
	virtual void data_handler(size_t bytes_received) { }

	//! IN buffer
	Post_buf post_buf;
	//! DATA buffer
	Data_buf data_buf;
};

/*! @brief %Request handling class for HTTP forms
 *
 * If you want to use UTF-8 encoding pass wchar_t as the template
 * argument. If you want to manually use iconv or use raw bytes,
 * then pass char or unsigned char as the template argument.
 *
 * @tparam char_type Character type for internal processing (wchar_t or char)
 * @tparam post_vec_type Vector type for storing file data
 * @tparam data_vec_type Vector type for storing DATA stream (when role == Role::filter)
 *
 * @see Request
 */
template <class char_type, typename post_val_type = std::basic_string<char_type>, typename data_buf_type = std::vector<uchar>>
class Form_request : public virtual Request<std::vector<uchar>, data_buf_type> {
protected:
	//! Structure containing all FastCGI HTTP session data
	http::Session<char_type, post_val_type> session;
	
	//! Handler for parsed PARAMS
	virtual bool params_handler(std::map<std::string, std::string> const& env) {
		for (auto const& e : env)
			session.parse_param(e);
		return true;
	}

	//! Handler for IN records
	virtual void in_handler(const uchar* data, size_t len) {
		session.fill_post(data, len);
		this->in_handler(len);
	}

	virtual void in_handler(size_t) { }
};

MOSH_FCGI_END


#endif
