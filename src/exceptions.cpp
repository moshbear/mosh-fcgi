//! \file exceptions.cpp Defines fastcgi++ exceptions member functions
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

#include <fastcgipp-mosh/exceptions.hpp>

#include <sstream>

Fastcgipp_m0sh::Exceptions::Param::Param(Fastcgipp_m0sh::Protocol::FullId id_): Request(id_)
{
	std::stringstream sstr;
	sstr << "Error in parameter code conversion in request #" << id.fcgiId << " of file descriptor #" << id.fd;
	msg = sstr.str();
}

Fastcgipp_m0sh::Exceptions::Stream::Stream(Fastcgipp_m0sh::Protocol::FullId id_): Request(id_)
{
	std::stringstream sstr;
	sstr << "Error in output stream code conversion in request #" << id.fcgiId << " of file descriptor #" << id.fd;
	msg = sstr.str();
}

Fastcgipp_m0sh::Exceptions::RecordOutOfOrder::RecordOutOfOrder(Fastcgipp_m0sh::Protocol::FullId id_, Protocol::RecordType expectedRecord_, Protocol::RecordType recievedRecord_)
	: Request(id_), expectedRecord(expectedRecord_), recievedRecord(recievedRecord_)
{
	std::stringstream sstr;
	sstr << "Error: Parameter of type " << Protocol::recordTypeLabels[recievedRecord] << " when type " << Protocol::recordTypeLabels[expectedRecord] << " was expected in request #" << id.fcgiId << " of file descriptor #" << id.fd;
	msg = sstr.str();
}

Fastcgipp_m0sh::Exceptions::SocketWrite::SocketWrite(int fd_, int erno_): Socket(fd_, erno_)
{
	std::stringstream sstr;
	sstr << "Error writing to socket #" << fd << " with errno=" << erno;
	msg = sstr.str();
}

Fastcgipp_m0sh::Exceptions::SocketRead::SocketRead(int fd_, int erno_): Socket(fd_, erno_)
{
	std::stringstream sstr;
	sstr << "Error reading from socket #" << fd << " with errno=" << erno;
	msg = sstr.str();
}

Fastcgipp_m0sh::Exceptions::Poll::Poll(int erno_): erno(erno_)
{
	std::stringstream sstr;
	sstr << "Error in poll with errno=" << erno;
	msg = sstr.str();
}
