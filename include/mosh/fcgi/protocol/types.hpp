//! @file  mosh/fcgi/protocol/types.hpp Types used for FastCGI protocol
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*               2007 Eddie                                                 *
*  Excerpts (C) 1996 Open Market, Inc. 
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


#ifndef MOSH_FCGI_PROTOCOL_TYPES_HPP
#define MOSH_FCGI_PROTOCOL_TYPES_HPP

#include <map>
#include <string>
#include <type_traits>
#include <cstdint>
#include <mosh/fcgi/bits/aligned.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @brief Defines aspects of the FastCGI %Protocol
 *
 * The %Protocol namespace defines the data structures and constants
 * used by the FastCGI protocol version 1. All data has been modelled
 * after the official FastCGI protocol specification located at
 * http://www.fastcgi.com/devkit/doc/fcgi-spec.html
 */
namespace protocol {


	//! The request ID of a FastCGI request
	typedef uint16_t Request_id;

	//! Typedef for aligned header bitstream
	typedef typename std::aligned_storage<1, 8>::type Header_data[8];

	/*! @brief  A full ID value for a FasTCGI request
	 *
	 * Because each FastCGI request has a RequestID and a file descriptor
	 * associated with it, this class defines an ID value that encompasses
	 * both. The file descriptor is stored internally as a 16 bit unsigned
	 * integer in order to keep the data structures size at 32 bits for
	 * optimized indexing.
	 */
	struct Full_id;
	
	//! Defines the types of records within the FastCGI protocol
	enum class Record_type : uint8_t
		{ invalid = 0, //!< Default value
		  
		  /* Application records */

		  /*! @brief FastCGI start request
		   *
		   * Sent by the Web server to start a request.
		   */
		  begin_request = 1,
		  /*! @brief FastCGI abort request
		   *
		   * Sent by the Web server to abort a request. This usually
		   * happens when the HTTP client prematurely closes its connection.
		   */
		  abort_request,
		  /*! @brief FastCGI end request 
		   *
		   * Sent by either the Web server or the application to end a request.
		   *
		   * This is used for both normal and error termination.
		   */
		  end_request,
		  //! @brief FastCGI per-request environment variables
		  params,
		  
		  /* Byte streams */

		  //! FastCGI stdin 
		  in,
		  //! FastCGI stdout 
		  out,
		  //! FastCGI stderr 
		  err, 
		  //! FastCGI filter data 
		  data,
		  
		  /* Management records */
		  
		  /*! @brief Management record query for specific variables
		   *
		   * Currently defined:
		   * @li FCGI_MAX_CONNS
		   * @li FCGI_MAX_REQS
		   * @li FCGI_MPXS_CONNS
		   */
		  get_values,
		  /*! @brief Management record response
		   */
		  get_values_result,
		  /*! Management record unknown type 
		   */
		  unknown_type, 
		};
	
	//! Defines the possible roles a FastCGI application may play
	enum class Role : uint16_t
		{ invalid = 0, //!< Default value
		   /*! This is the basic FastCGI role, and corresponds to the
		    * simple functionality offered by CGI today.
		    */
		  responder = 1,
		  /*! The FastCGI program performs an access control decision
		   *  for the request (such as performing a username/password database
		   *  lookup).
		   */
		  authorizer,
		  /*! The FastCGI application filters the requested Web server file
		   *  before sending it to the client.
		   */
		  filter,
		};

	//! Possible statuses a request may declare when complete
	enum class Protocol_status : uint8_t
	{
	  /*! Normal end of request.
	   */
	  request_complete = 0,
	  /*! Rejecting a new request because the application is unable to
	   *  handle concurrent requests over one connection
	   */
	  cant_mpx_conn,
	  /*! Rejecting a new request because the application has run out of
	   *  some resource.
	   */
	  overloaded,
	  /*! Rejecting a new request because the the Web server has specified a
	   *  role that is unknown to the application.
	   */
	  unknown_role,
	};

	/*! @brief Data structure used as the header for FastCGI records
	 *
	 * This structure defines the header used in FastCGI records. It can be casted 
	 * to and from raw 8 byte blocks of data and transmitted/received as is. The
	 * endianess and order of data is kept correct through the accessor member functions.
	 */
	class Header;

	/*! @brief Data structure used as the body for FastCGI records with a RecordType of begin_request
	 * 
	 * This structure defines the body used in FastCGI begin_request records. It can be casted 
	 * from raw 8 byte blocks of data and received as is. A begin_request record is received
	 * when the other side wished to make a new request.
	 */
	class Begin_request;

	/*! @brief Data structure used as the body for FastCGI records with a Record_type of unknown_type
	 * 
	 * This structure defines the body used in FastCGI unknown_type records. It can be casted 
	 * to raw 8 byte blocks of data and transmitted as is. An unknown_type record is sent as
	 * a reply to record types that are not recognized.
	 */
	class Unknown_type;
		
	/*! @brief Data structure used as the body for FastCGI records with a Record_type of end_request
	 * 
	 * This structure defines the body used in FastCGI end_request records. It can be casted 
	 * to raw 8 byte blocks of data and transmitted as is. An end_request record is sent when
	 * this side wishes to terminate a request. This can be simply because it is complete or
	 * because of a problem.
	 */
	class End_request;

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
	struct Message;

}

MOSH_FCGI_END

#include <mosh/fcgi/protocol/gs.hpp> // do not move this to before namespace definition

#endif
