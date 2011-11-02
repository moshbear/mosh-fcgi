//! \file protocol.hpp Defines FasTCGI protocol
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


#ifndef MOSH_FCGI_PROTOCOL_MESSAGE_HPP
#define MOSH_FCGI_PROTOCOL_MESSAGE_HPP

#include <cstddef>
#include <boost/shared_array.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/* @brief Defines aspects of the FastCGI %Protocol
 * The %Protocol namespace defines the data structures and constants
 * used by the FastCGI protocol version 1. All data has been modelled
 * after the official FastCGI protocol specification located at
 * http://www.fastcgi.com/devkit/doc/fcgi-spec.html
 */
namespace protocol {

	//! Data structure used to pass messages within the fastcgi++ task management system
	/*!
	 * This data structure is crucial to all operation in the FastCGI library as all
	 * data passed to requests must be encapsulated in this data structure. A type value
	 * of 0 means that the message is a FastCGI record and will be processed at a low
	 * level by the library. Any other type value and the message will be passed up to
	 * the user to be processed. The data may contain any data that can be casted to/from
	 * a raw character array. The size obviously represents the exact size of the data
	 * section.
	 */
	struct Message
	{
		//! Type of message. A 0 means FastCGI record. Anything else is open.
		int type;
		//! Size of the data section.
		size_t size;
		//! Pointer to the raw data being passed along with the message.
		boost::shared_array<char> data;
	};
}


MOSH_FCGI_END

#endif
