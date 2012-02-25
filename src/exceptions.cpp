//! @file exceptions.cpp Defines mosh-fcgi exceptions member functions
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

#include <mosh/fcgi/exceptions.hpp>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <sstream>
#include <mosh/fcgi/bits/namespace.hpp>


MOSH_FCGI::exceptions::Param::Param(MOSH_FCGI::protocol::Full_id id_): Request(id_)
{
	std::stringstream sstr;
	sstr << "Error in parameter code conversion in request #" << id.fcgi_id << " of file descriptor #" << id.fd;
	msg = sstr.str();
}

MOSH_FCGI::exceptions::Stream::Stream(MOSH_FCGI::protocol::Full_id id_): Request(id_)
{
	std::stringstream sstr;
	sstr << "Error in output stream code conversion in request #" << id.fcgi_id << " of file descriptor #" << id.fd;
	msg = sstr.str();
}

MOSH_FCGI::exceptions::Record_out_of_order::Record_out_of_order(MOSH_FCGI::protocol::Full_id id_, protocol::Record_type expected_record_, protocol::Record_type recieved_record_)
	: Request(id_), expected_record(expected_record_), recieved_record(recieved_record_)
{
	std::stringstream sstr;
	sstr << "Error: Parameter of type " << protocol::record_type_labels[static_cast<size_t>(recieved_record)]
		<< " when type " << protocol::record_type_labels[static_cast<size_t>(expected_record)]
		<< " was expected in request #" << id.fcgi_id << " of file descriptor #" << id.fd;
	msg = sstr.str();
}

MOSH_FCGI::exceptions::Socket_write::Socket_write(int fd_, int erno_): Socket(fd_, erno_)
{
	std::stringstream sstr;
	sstr << "Error writing to socket #" << fd << " with errno=" << erno;
	msg = sstr.str();
}

MOSH_FCGI::exceptions::Socket_read::Socket_read(int fd_, int erno_): Socket(fd_, erno_)
{
	std::stringstream sstr;
	sstr << "Error reading from socket #" << fd << " with errno=" << erno;
	msg = sstr.str();
}

MOSH_FCGI::exceptions::Poll::Poll(int erno_): erno(erno_)
{
	std::stringstream sstr;
	sstr << "Error in poll with errno=" << erno;
	msg = sstr.str();
}
