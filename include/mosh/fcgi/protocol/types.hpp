//! \file protocol/types.hpp Type declarations for protocol
/***************************************************************************
* Copyright (C) 2007 Eddie                                                 *
*               2011 m0shbear                                              *
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


#ifndef MOSH_FCGI_PROTOCOL_TYPES_HPP
#define MOSH_FCGI_PROTOCOL_TYPES_HPP

#include <map>
#include <string>
#include <cstdint>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/* @brief Defines aspects of the FastCGI %Protocol
 * The %Protocol namespace defines the data structures and constants
 * used by the FastCGI protocol version 1. All data has been modelled
 * after the official FastCGI protocol specification located at
 * http://www.fastcgi.com/devkit/doc/fcgi-spec.html
 */
namespace protocol {

	//! The request ID of a FastCGI request
	typedef uint16_t Request_id;

	//! Typedef for aligned header bitstream
	typedef char MOSH_FCGI_ALIGNEDAS_N(8, Header_data[8]);

	/*! @brief  A full ID value for a FasTCGI request!
	 * Because each FastCGI request has a RequestID and a file descriptor
	 * associated with it, this class defines an ID value that encompasses
	 * both. The file descriptor is stored internally as a 16 bit unsigned
	 * integer in order to keep the data structures size at 32 bits for
	 * optimized indexing.
	 */
	struct Full_id;
	
	//! Defines the types of records within the FastCGI protocol
	enum class Record_type : uint8_t
		{ invalid = 0,
		  begin_request = 1, abort_request, end_request,
		  params, in, out, err, data,
		  get_values, get_values_result, unknown_type 
		};
	
	//! Defines the possible roles a FastCGI application may play
	enum class Role : uint16_t
		{ invalid = 0,
		  responder = 1, authorizer, filter
		};

	//! Possible statuses a request may declare when complete
	enum class Protocol_status : uint8_t
		{ request_complete = 0, cant_mpx_conn, overloaded, unknown_role };

	/*! @brief Data structure used as the header for FastCGI records!
	 * This structure defines the header used in FastCGI records. It can be casted 
	 * to and from raw 8 byte blocks of data and transmitted/received as is. The
	 * endianess and order of data is kept correct through the accessor member functions.
	 */
	class Header;

	/*! @brief Data structure used as the body for FastCGI records with a RecordType of begin_request
	 * This structure defines the body used in FastCGI begin_request records. It can be casted 
	 * from raw 8 byte blocks of data and received as is. A begin_request record is received
	 * when the other side wished to make a new request.
	 */
	class Begin_request;

	/*! @brief Data structure used as the body for FastCGI records with a Record_type of unknown_type
	 * This structure defines the body used in FastCGI unknown_type records. It can be casted 
	 * to raw 8 byte blocks of data and transmitted as is. An unknown_type record is sent as
	 * a reply to record types that are not recognized.
	 */
	class Unknown_type;
		
	/*! @brief Data structure used as the body for FastCGI records with a Record_type of end_request
	 * This structure defines the body used in FastCGI end_request records. It can be casted 
	 * to raw 8 byte blocks of data and transmitted as is. An end_request record is sent when
	 * this side wishes to terminate a request. This can be simply because it is complete or
	 * because of a problem.
	 */
	class End_request;

	/*! @brief Used for the reply of FastCGI management records of type get_values
	 * This class template is an efficient tool for replying to get_values management
	 * records. The structure represents a complete record (body+header) of a name-value pair to be
	 * sent as a reply to a management value query. The templating allows the structure
	 * to be exactly the size that is needed so it can be casted to raw data and transmitted
	 * as is. Note that the name and value lengths are left as single bytes so they are limited
	 * in range from 0-127.
	 *
	 * @tparam name_len Length of name in bytes (0-127). Null terminator not included.
	 * @tparam value_len Length of value in bytes (0-127). Null terminator not included.
	 * @tparam padding_len Length of padding at the end of the record. This is needed to keep
	 * the record size a multiple of chunkSize.
	 */
	template <int name_len, int value_len, int padding_len>
	struct Management_reply;

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
	struct Message;
}

MOSH_FCGI_END

#endif
