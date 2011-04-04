//! \file exceptions.hpp Defines fastcgi++ exceptions
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


#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>
#include <string>

#include <fastcgipp-mosh/protocol.hpp>

//! Topmost namespace for the fastcgi++ library
namespace Fastcgipp_m0sh
{
//! Namespace that defines fastcgi++ related exceptions
namespace Exceptions
{
//! General fastcgi++ exception
class Exception: public std::exception
{
public:
	virtual const char* what() const throw() = 0;
	~Exception() throw() {}
};

//! General fastcgi++ request exception
class Request: public Exception
{
public:
	//! Sole Constructor
	/*!
	 * @param[in] id_ ID value for the request that generated the exception
	 */
	Request(Protocol::FullId id_): id(id_) { }
	~Request() throw() {}

	virtual const char* what() const throw() = 0;
	Protocol::FullId getId() const throw() {
		return id;
	}
protected:
	//! ID value for the request that generated the exception
	Protocol::FullId id;
};

//! %Exception for parameter decoding errors
class Param: public Request
{
public:
	//! Sole Constructor
	/*!
	 * @param[in] id_ ID value for the request that generated the exception
	 */
	Param(Protocol::FullId id_);
	~Param() throw() {}
	virtual const char* what() const throw() {
		return msg.c_str();
	}
private:
	//! Error message associated with the exception
	std::string msg;
};

//! %Exception for output stream processing
class Stream: public Request
{
public:
	//! Sole Constructor
	/*!
	 * @param[in] id_ ID value for the request that generated the exception
	 */
	Stream(Protocol::FullId id_);
	~Stream() throw() {}
	virtual const char* what() const throw() {
		return msg.c_str();
	}
private:
	//! Error message associated with the exception
	std::string msg;
};

//! %Exception for reception of records out of order
class RecordOutOfOrder: public Request
{
public:
	//! Sole Constructor
	/*!
	 * @param[in] id_ ID value for the request that generated the exception
	 * @param[in] expectedRecord_ Type of record that was expected
	 * @param[in] recievedRecord_ Type of record that was recieved
	 */
	RecordOutOfOrder(Protocol::FullId id_, Protocol::RecordType expectedRecord_, Protocol::RecordType recievedRecord_);
	~RecordOutOfOrder() throw() {}
	virtual const char* what() const throw() {
		return msg.c_str();
	}
	Protocol::RecordType getExpectedRecord() const throw() {
		return expectedRecord;
	}
	Protocol::RecordType getRecievedRecord() const throw() {
		return recievedRecord;
	}
private:
	//! Type of record that was expected
	Protocol::RecordType expectedRecord;
	//! Type of record that was recieved
	Protocol::RecordType recievedRecord;
	//! Error message associated with the exception
	std::string msg;
};

//! General exception for socket related errors
class Socket: public Exception
{
public:
	//! Sole Constructor
	/*!
	 * @param[in] fd_ File descriptor of socket
	 * @param[in] erno_ Associated errno
	 */
	Socket(int fd_, int erno_): fd(fd_), erno(erno_) { }
	~Socket() throw() {}
	virtual const char* what() const throw() = 0;
	int getFd() const throw() {
		return fd;
	}
	int getErrno() const throw() {
		return erno;
	}
protected:
	//! File descriptor of socket
	int fd;
	//! Associated errno
	int erno;
};

//! %Exception for write errors to sockets
class SocketWrite: public Socket
{
public:
	//! Sole Constructor
	/*!
	 * @param[in] fd_ File descriptor of socket
	 * @param[in] erno_ Associated errno
	 */
	SocketWrite(int fd_, int erno_);
	~SocketWrite() throw() {}
	virtual const char* what() const throw() {
		return msg.c_str();
	}
private:
	//! Error message associated with the exception
	std::string msg;
};

//! %Exception for read errors to sockets
class SocketRead: public Socket
{
public:
	//! Sole Constructor
	/*!
	 * @param[in] fd_ File descriptor of socket
	 * @param[in] erno_ Associated errno
	 */
	SocketRead(int fd_, int erno_);
	~SocketRead() throw() {}
	virtual const char* what() const throw() {
		return msg.c_str();
	}
private:
	//! Error message associated with the exception
	std::string msg;
};

//! %Exception for poll() errors
class Poll: public Exception
{
public:
	//! Sole Constructor
	/*!
	 * @param[in] erno_ Associated errno
	 */
	Poll(int erno_);
	~Poll() throw() {}
	virtual const char* what() const throw() {
		return msg.c_str();
	}
	int getErno() const throw() {
		return erno;
	}
private:
	//! Associated errno
	int erno;
	//! Error message associated with the exception
	std::string msg;
};

//! %Exception for partial content
class Partial: public Exception
{
public:
	Partial(const std::string& str) : _str(str) { }
	~Partial() throw() {}
	virtual const char* what() const throw() { return _str.c_str(); }
private:
	std::string _str;
};
}
}

#endif
