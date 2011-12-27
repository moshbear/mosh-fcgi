//! @file protocol/header.hpp FastCGI header
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


#ifndef MOSH_FCGI_PROTOCOL_HEADER_HPP
#define MOSH_FCGI_PROTOCOL_HEADER_HPP

#include <cstdint>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/gs.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/* @brief Defines aspects of the FastCGI %Protocol
 * The %Protocol namespace defines the data structures and constants
 * used by the FastCGI protocol version 1. All data has been modelled
 * after the official FastCGI protocol specification located at
 * http://www.fastcgi.com/devkit/doc/fcgi-spec.html
 */
namespace protocol {

#pragma pack(push, 1)
	/*! @brief Data structure used as the header for FastCGI records!
	 * This structure defines the header used in FastCGI records. It can be casted 
	 * to and from raw 8 byte blocks of data and transmitted/received as is. The
	 * endianess and order of data is kept correct through the accessor member functions.
	 */
	class Header {
	public:	
		Header() { }
		Header(uint8_t _version_, Record_type _type_, uint16_t _reqid_, uint16_t _clen_, uint8_t _padlen_) {
			version() = _version_;
			type() = _type_;
			request_id() = _reqid_;
			content_length() = _clen_;
			padding_length() = _padlen_;
		}

		/*! @name Getter-setters for version field of record header
		 */
		//@{
		_gs_u8 version() { return _gs_u8(_version); }
		_g_u8 version() const { return _g_u8(_version); }
		//@}
		/*! @name Getter-setters for record type in header
		*/
		//@{
		_gs_type type() { return _gs_type(_type); }
		_g_type type() const { return _g_type(_type); }
		//@}
		/*! @name Getter-setters for request ID field in record header
		 */
		//@{
		_gs_u16 request_id() { return _gs_u16(_request_id); }
		_g_u16 request_id() const { return _g_u16(_request_id); }
		//@}
		/*! @name Getter-setters for content length field in record header
		 */
		//@{
		_gs_u16 content_length() { return _gs_u16(_content_length); }
		_g_u16 content_length() const { return _g_u16(_content_length); }
		//@}
		/*! @name Getter-setters for padding length in record header
		 */
		//@{
		_gs_u8 padding_length() { return _gs_u8(_padding_length); }
		
		_g_u8 padding_length() const { return _g_u8(_padding_length); }
		//@}
	private:
		//! FastCGI version number
		uint8_t _version;
		//! Record type
		uint8_t _type;
		uint16_t _request_id;
		uint16_t _content_length;
		//! Length of record padding
		uint8_t _padding_length;
		//! Reseved for future use and header padding
		uint8_t _reserved;
	};
#pragma pack(pop)

}

MOSH_FCGI_END

#endif
