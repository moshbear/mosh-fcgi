//! \file protocol.hpp Defines FastCGI protocol
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


#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <map>
#include <string>
extern "C" {
#include <netinet/in.h>
#include <arpa/inet.h>
}
#include <fastcgipp-mosh/bits/stdint.hpp>
#include <boost/shared_array.hpp>

//! Topmost namespace for the fastcgi++ library
namespace Fastcgipp_m0sh
{
	namespace Protocol
	{
		//! The request ID of a FastCGI request
		typedef uint16_t RequestId;
		
		//! A full ID value for a FastCGI request
		/*!
		 * Because each FastCGI request has a RequestID and a file descriptor
		 * associated with it, this class defines an ID value that encompasses
		 * both. The file descriptor is stored internally as a 16 bit unsigned
		 * integer in order to keep the data structures size at 32 bits for
		 * optimized indexing.
		 */
		struct FullId
		{
			//! Construct from a FastCGI RequestID and a file descriptor
			/*!
			 * The constructor builds upon a RequestID and the file descriptor
			 * it is communicating through.
			 *
			 * @param  [in] fcgiId_ The FastCGI request ID
			 * @param [in] fd_ The file descriptor
			 */
			FullId(RequestId fcgiId_, int fd_)
			: fcgiId(fcgiId_), fd(fd_)
			{ } 
			FullId() { }
			//! FastCGI Request ID
			RequestId fcgiId;
			//! Associated File Descriptor
			uint16_t fd;
		};
		
		//! Defines the types of records within the FastCGI protocol
		enum RecordType {
			BEGIN_REQUEST = 1,
			ABORT_REQUEST = 2,
			END_REQUEST = 3,
			PARAMS = 4,
			IN = 5,
			OUT = 6,
			ERR = 7,
			DATA = 8,
			GET_VALUES = 9,
			GET_VALUES_RESULT = 10,
			UNKNOWN_TYPE=11
		};
		
		//! Defines text labels for the RecordType values
		extern char* recordTypeLabels[];
	}
}
		

#include <fastcgipp-mosh/exceptions.hpp>

//! Topmost namespace for the fastcgi++ library
namespace Fastcgipp_m0sh
{
	//! Defines aspects of the FastCGI %Protocol
	/*!
	 * The %Protocol namespace defines the data structures and constants
	 * used by the FastCGI protocol version 1. All data has been modelled
	 * after the official FastCGI protocol specification located at
	 * http://www.fastcgi.com/devkit/doc/fcgi-spec.html
	 */
	namespace Protocol
	{
		//! The version of the FastCGI protocol that this adheres to
		const int version=1;

		//! All FastCGI records will be a multiple of this many bytes
		const int chunkSize=8;

		//! Defines the possible roles a FastCGI application may play
		enum Role {
			RESPONDER = 1,
			AUTHORIZER = 2,
			FILTER = 3
		};

		//! Possible statuses a request may declare when complete
		enum ProtocolStatus {
			REQUEST_COMPLETE = 0,
			CANT_MPX_CONN = 1,
			OVERLOADED = 2,
			UNKNOWN_ROLE = 3
		};

		//!Compare between two FullId variables
		/*!
		 * This comparator casts the structures as 32 bit integers and compares them as such.
		 */
		inline bool operator>(const FullId& x, const FullId& y) {
			return *reinterpret_cast<const uint32_t*>(&x.fcgiId) > *reinterpret_cast<const uint32_t*>(&y.fcgiId);
		}

		//!Compare between two FullId variables
		/*!
		 * This comparator casts the structures as 32 bit integers and compares them as such.
		 */
		inline bool operator<(const FullId& x, const FullId& y) {
			return *reinterpret_cast<const uint32_t*>(&x.fcgiId) < *reinterpret_cast<const uint32_t*>(&y.fcgiId);
		}

		//!Compare between two FullId variables
		/*!
		 * This comparator casts the structures as 32 bit integers and compares them as such.
		 */
		inline bool operator==(const FullId& x, const FullId& y) {
			return *reinterpret_cast<const uint32_t*>(&x.fcgiId) == *reinterpret_cast<const uint32_t*>(&y.fcgiId);
		}


		//!Data structure used as the header for FastCGI records
		/*!
		 * This structure defines the header used in FastCGI records. It can be casted 
		 * to and from raw 8 byte blocks of data and transmitted/received as is. The
		 * endianess and order of data is kept correct through the accessor member functions.
		 */
		class Header
		{
		public:
			//!Set the version field of the record header
			/*!
			 * @param[in] version_ FastCGI protocol version number
			 */
			void setVersion(uint8_t version_) {
				version=version_;
			}

			//!Get the version field of the record header
			/*!
			 * @return version FastCGI protocol version number
			 */
			int getVersion() const {
				return version;
			}

			//!Set the record type in the header
			/*!
			 * @param[in] type_ Record type
			 */
			void setType(RecordType type_) {
				type=static_cast<uint8_t>(type_);
			}

			//!Get the record type in the header
			/*!
			 * @return Record type
			 */
			RecordType getType() const {
				return static_cast<RecordType>(type);
			}

			//!Set the request ID field in the record header
			/*!
			 * @param[in] requestId_ The records request ID
			 */
			void setRequestId(RequestId requestId_) {
				*reinterpret_cast<uint16_t*>(&requestIdB1)=ntohs(requestId_);
			}

			//!Get the request ID field in the record header
			/*!
			 * @return The records request ID
			 */
			RequestId getRequestId() const {
				return ntohs(*reinterpret_cast<const uint16_t*>(&requestIdB1));
			}

			//!Set the content length field in the record header
			/*!
			 * @param[in] contentLength_ The records content length
			 */
			void setContentLength(uint16_t contentLength_) {
				*reinterpret_cast<uint16_t*>(&contentLengthB1) = ntohs(contentLength_);
			}

			//!Get the content length field in the record header
			/*!
			 * @return The records content length
			 */
			int getContentLength() const {
				return ntohs(*reinterpret_cast<const uint16_t*>(&contentLengthB1));
			}

			//!Set the padding length field in the record header
			/*!
			 * @param[in] paddingLength_ The records padding length
			 */
			void setPaddingLength(uint8_t paddingLength_) {
				paddingLength=paddingLength_;
			}

			//!Get the padding length field in the record header
			/*!
			 * @return The records padding length
			 */
			int getPaddingLength() const {
				return paddingLength;
			}
		private:
			//! FastCGI version number
			uint8_t version;
			//! Record type
			uint8_t type;
			//! Request ID most significant byte
			uint8_t requestIdB1;
			//! Request ID least significant byte
			uint8_t requestIdB0;
			//! Content length most significant byte
			uint8_t contentLengthB1;
			//! Content length least significant byte
			uint8_t contentLengthB0;
			//! Length of record padding
			uint8_t paddingLength;
			//! Reseved for future use and header padding
			uint8_t reserved;
		};

		
		//!Data structure used as the body for FastCGI records with a RecordType of BEGIN_REQUEST
		/*!
		 * This structure defines the body used in FastCGI BEGIN_REQUEST records. It can be casted 
		 * from raw 8 byte blocks of data and received as is. A BEGIN_REQUEST record is received
		 * when the other side wished to make a new request.
		 */
		class BeginRequest
		{
		public:
			//!Get the role field from the record body
			/*!
			 * @return The expected Role that the request will play
			 */
			Role getRole() const {
				return static_cast<Role>(ntohs(*reinterpret_cast<const uint16_t*>(&roleB1)));
			}

			//!Get keep alive value from the record body
			/*!
			 * If this value is false, the socket should be closed on our side when the request is complete.
			 * If true, the other side will close the socket when done and potentially reuse the socket and
			 * multiplex other requests on it.
			 *
			 * @return Boolean value as to whether or not the connection is kept alive
			 */
			bool getKeepConn() const {
				return flags & keepConnBit;
			}
		private:
			//! Flag bit representing the keep alive value
			static const int keepConnBit = 1;

			//! Role value most significant byte
			uint8_t roleB1;
			//! Role value least significant byte
			uint8_t roleB0;
			//! Flag value
			uint8_t flags;
			//! Reseved for future use and body padding
			uint8_t reserved[5];
		};

		//!Data structure used as the body for FastCGI records with a RecordType of UNKNOWN_TYPE
		/*!
		 * This structure defines the body used in FastCGI UNKNOWN_TYPE records. It can be casted 
		 * to raw 8 byte blocks of data and transmitted as is. An UNKNOWN_TYPE record is sent as
		 * a reply to record types that are not recognized.
		 */
		class UnknownType
		{
		public:
			//!Set the record type that is unknown
			/*!
			 * @param[in] type_ The unknown record type
			 */
			void setType(RecordType type_) {
				type=static_cast<uint8_t>(type_);
			}
		private:
			//! Unknown record type
			uint8_t type;
			//! Reseved for future use and body padding
			uint8_t reserved[7];
		};

		//!Data structure used as the body for FastCGI records with a RecordType of END_REQUEST
		/*!
		 * This structure defines the body used in FastCGI END_REQUEST records. It can be casted 
		 * to raw 8 byte blocks of data and transmitted as is. An END_REQUEST record is sent when
		 * this side wishes to terminate a request. This can be simply because it is complete or
		 * because of a problem.
		 */
		class EndRequest
		{
		public:
			//!Set the requests return value
			/*!
			 * This is an integer value representing what would otherwise be the return value in a
			 * normal CGI application.
			 *
			 * @param[in] status The return value
			 */
			void setAppStatus(int status) {
				*reinterpret_cast<int*>(&appStatusB3)=ntohl(status);
			}

			//!Set the reason for termination
			/*!
			 * This value is one of ProtocolStatus and represents the reason for termination.
			 *
			 * @param[in] status The requests status
			 */
			void setProtocolStatus(ProtocolStatus status) {
				protocolStatus=static_cast<uint8_t>(status);
			}
		private:
			//! Return value most significant byte
			uint8_t appStatusB3;
			//! Return value second most significant byte
			uint8_t appStatusB2;
			//! Return value third most significant byte
			uint8_t appStatusB1;
			//! Return value least significant byte
			uint8_t appStatusB0;
			//! Requests Status
			uint8_t protocolStatus;
			//! Reseved for future use and body padding
			uint8_t reserved[3];
		};

		//!Process the body of a FastCGI parameter record
		/*!
		 * Takes the body of a FastCGI record of type PARAMETER and parses  it. You end
		 * up with a pointer/size for both the name and value of the parameter.
		 *
		 * @param[in] data Pointer to the record body
		 * @param[out] name Reference to a pointer that will be pointed to the first byte of the parameter name
		 * @param[out] nameSize Reference to a value to will be given the size in bytes of the parameter name
		 * @param[out] value Reference to a pointer that will be pointed to the first byte of the parameter value
		 * @param[out] valueSize Reference to a value to will be given the size in bytes of the parameter value
		 */
		bool processParamHeader(const char* data, size_t dataSize,
					const char*& name, size_t& nameSize,
					const char*& value, size_t& valueSize);

		
		//!Used for the reply of FastCGI management records of type GET_VALUES
		/*!
		 * This class template is an efficient tool for replying to GET_VALUES management
		 * records. The structure represents a complete record (body+header) of a name-value pair to be
		 * sent as a reply to a management value query. The templating allows the structure
		 * to be exactly the size that is needed so it can be casted to raw data and transmitted
		 * as is. Note that the name and value lengths are left as single bytes so they are limited
		 * in range from 0-127.
		 *
		 * @tparam NAMELENGTH Length of name in bytes (0-127). Null terminator not included.
		 * @tparam VALUELENGTH Length of value in bytes (0-127). Null terminator not included.
		 * @tparam PADDINGLENGTH Length of padding at the end of the record. This is needed to keep
		 * the record size a multiple of chunkSize.
		 */
		template<int NAMELENGTH, int VALUELENGTH, int PADDINGLENGTH>
		struct ManagementReply
		{
		private:
			//! Management records header
			Header header;
			//! Length in bytes of name
			uint8_t nameLength;
			//! Length in bytes of value
			uint8_t valueLength;
			//! Name data
			uint8_t name[NAMELENGTH];
			//! Value data
			uint8_t value[VALUELENGTH];
			//! Padding data
			uint8_t padding[PADDINGLENGTH];
		public:
			//! Construct the record based on the name data and value data
			/*!
			 * A full record is constructed from the name-value data. After
			 * construction the structure can be casted to raw data and transmitted
			 * as is. The size of the data arrays pointed to by name_ and value_ are
			 * assumed to correspond with the NAMELENGTH and PADDINGLENGTH template
			 * parameters passed to the class.
			 *
			 * @param[in] name_ Pointer to name data
			 * @param[in] value_ Pointer to value data
			 */
			ManagementReply(const char* name_, const char* value_)
			: nameLength(NAMELENGTH), valueLength(VALUELENGTH)
			{
				for(int i=0; i<NAMELENGTH; i++) name[i]=*(name_+i);
				for(int i=0; i<VALUELENGTH; i++) value[i]=*(value_+i);
				header.setVersion(version);
				header.setType(GET_VALUES_RESULT);
				header.setRequestId(0);
				header.setContentLength(NAMELENGTH+VALUELENGTH);
				header.setPaddingLength(PADDINGLENGTH);
			}
		};

		//! Reply record that will be sent when asked the maximum allowed file descriptors open at a time
		extern ManagementReply<14, 2, 8> maxConnsReply;
		//! Reply record that will be sent when asked the maximum allowed requests at a time
		extern ManagementReply<13, 2, 1> maxReqsReply;
		//! Reply record that will be sent when asked if requests can be multiplexed over a single connections
		extern ManagementReply<15, 1, 8> mpxsConnsReply;
	}

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

#endif
