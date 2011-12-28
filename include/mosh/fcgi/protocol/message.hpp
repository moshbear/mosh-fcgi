//! @file protocol/message.hpp mosh-fcgi message
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


#ifndef MOSH_FCGI_PROTOCOL_MESSAGE_HPP
#define MOSH_FCGI_PROTOCOL_MESSAGE_HPP

#include <cstddef>
#include <memory>
#include <mosh/fcgi/bits/array_deleter.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace protocol {

	/*! @brief Data structure used to pass messages within the mosh-fcgi task management system
	 *
	 * This data structure is crucial to all operation in the FastCGI library as all
	 * data passed to requests must be encapsulated in this data structure. A type value
	 * of 0 means that the message is a FastCGI record and will be processed at a low
	 * level by the library. Any other type value and the message will be passed up to
	 * the user to be processed. The data may contain any data that can be casted to/from
	 * a raw character array. The size obviously represents the exact size of the data
	 * section.
	 */
	struct Message {
		//! Type of message. A 0 means FastCGI record. Anything else is open.
		int type;
		//! Size of the data section.
		size_t size;
		//! Pointer to the raw data being passed along with the message.
		std::shared_ptr<char> data;
		//! Default constructor. Initializes data's Deleter.
		Message() : data(0, Array_deleter<char>()) { }
	};
}


MOSH_FCGI_END

#endif
