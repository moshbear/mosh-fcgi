//! @file  mosh/fcgi/exceptions.hpp Defines mosh-fcgi exceptions
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

#ifndef MOSH_FCGI_EXCEPTIONS_HPP
#define MOSH_FCGI_EXCEPTIONS_HPP

#include <exception>
#include <string>

#include <mosh/fcgi/bits/namespace.hpp>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>


MOSH_FCGI_BEGIN

namespace exceptions {

//! General mosh-fcgi exception
class Exception: public std::exception {
public:
	virtual const char* what() const throw() = 0;
	~Exception() throw() {}
};

//! General mosh-fcgi request exception
class Request: public Exception {
public:
	/*! @brief Sole Constructor
	 * @param[in] id ID value for the request that generated the exception
	 */
	Request(protocol::Full_id id): id(id) { }
	~Request() throw() {}

	virtual const char* what() const throw() = 0;
	protocol::Full_id getid() const throw() { return id; }
protected:
	//! ID value for the request that generated the exception
	protocol::Full_id id;
};

//! %Exception for parameter decoding errors
class Param: public Request {
public:
	/*! @brief Sole Constructor
	 * @param[in] id ID value for the request that generated the exception
	 */
	Param(protocol::Full_id id);
	~Param() throw() {}
	virtual const char* what() const throw() { return msg.c_str(); }
private:
	//! Error message associated with the exception
	std::string msg;
};

//! %Exception for output stream processing
class Stream: public Request {
public:
	/*! @brief Sole Constructor
	 * @param[in] id ID value for the request that generated the exception
	 */
	Stream(protocol::Full_id id);
	~Stream() throw() {}
	virtual const char* what() const throw() { return msg.c_str(); }
private:
	//! Error message associated with the exception
	std::string msg;
};

//! %Exception for reception of records out of order
class Record_out_of_order: public Request {
public:
	/*! @brief Sole Constructor
	 * @param[in] id ID value for the request that generated the exception
	 * @param[in] expected_record Type of record that was expected
	 * @param[in] recieved_record Type of record that was recieved
	 */
	Record_out_of_order(protocol::Full_id id, protocol::Record_type expected_record, protocol::Record_type recieved_record);
	~Record_out_of_order() throw() {}
	virtual const char* what() const throw() { return msg.c_str(); }
	protocol::Record_type get_expected_record() const throw() { return expected_record;	}
	protocol::Record_type get_recieved_record() const throw() { return recieved_record;	}
private:
	//! Type of record that was expected
	protocol::Record_type expected_record;
	//! Type of record that was recieved
	protocol::Record_type recieved_record;
	//! Error message associated with the exception
	std::string msg;
};

//! General exception for socket related errors
class Socket: public Exception {
public:
	/*! @brief Sole Constructor
	 * @param[in] fd File descriptor of socket
	 * @param[in] erno Associated errno
	 */
	Socket(int fd, int erno): fd(fd), erno(erno) { }
	~Socket() throw() {}
	virtual const char* what() const throw() = 0;
	int getfd() const throw() { return fd; }
	int get_errno() const throw() { return erno; }
protected:
	//! File descriptor of socket
	int fd;
	//! Associated errno
	int erno;
};

//! %Exception for write errors to sockets
class Socket_write: public Socket {
public:
	/*! @brief Sole Constructor
	 * @param[in] fd File descriptor of socket
	 * @param[in] erno Associated errno
	 */
	Socket_write(int fd, int erno);
	~Socket_write() throw() {}
	virtual const char* what() const throw() { return msg.c_str(); }
private:
	//! Error message associated with the exception
	std::string msg;
};

//! %Exception for read errors to sockets
class Socket_read: public Socket {
public:
	/*! @brief Sole Constructor
	 * @param[in] fd File descriptor of socket
	 * @param[in] erno Associated errno
	 */
	Socket_read(int fd, int erno);
	~Socket_read() throw() {}
	virtual const char* what() const throw() { return msg.c_str(); }
private:
	//! Error message associated with the exception
	std::string msg;
};

//! %Exception for poll() errors
class Poll: public Exception {
public:
	/*! @brief Sole Constructor
	 * @param[in] erno Associated errno
	 */
	Poll(int erno);
	~Poll() throw() {}
	virtual const char* what() const throw() {
		return msg.c_str();
	}
	int get_errno() const throw() {
		return erno;
	}
private:
	//! Associated errno
	int erno;
	//! Error message associated with the exception
	std::string msg;
};

//! %Exception for partial content
class Partial: public Exception {
public:
	Partial(const std::string& str) : str(str) { }
	~Partial() throw() {}
	virtual const char* what() const throw() { return str.c_str(); }
private:
	std::string str;
};

}

MOSH_FCGI_END

#endif
