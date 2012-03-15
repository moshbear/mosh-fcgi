//! @file  mosh/fcgi/fcgistream.hpp Defines the Fcgistream stream and stream buffer classes
/***************************************************************************
* Copyright (C) 2011-2 m0shbear                                            *
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

#ifndef MOSH_FCGI_FCGISTREAM_HPP
#define MOSH_FCGI_FCGISTREAM_HPP

#include <string>
#include <ostream>
#include <ios>
#include <istream>
#include <memory>

#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/transceiver.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! Stream class for output of client data through FastCGI
/*!
 * This class is derived from std::basic_ostream<unsigned char>. It acts just
 * the same as any stream does with the added feature of the dump() function.
 *
 */
class Fcgistream : public std::basic_ostream<uchar> {
public:
	Fcgistream();
	virtual ~Fcgistream();
	//! Arguments passed directly to Fcgibuf::set()
	void set(protocol::Full_id id, Transceiver& transceiver, protocol::Record_type type);
	//! 
	/*! @name Dumpers
	 */
	//@{
	/*! @brief Dumps raw data directly into the FastCGI protocol
	 *
	 * This function exists as a mechanism to dump raw data out the stream bypassing
	 * the stream buffer. 
	 *
	 * @param[in] data Pointer to first byte of data to send
	 * @param[in] size Size in bytes of data to be sent
	 */
	void dump(const uchar* data, size_t size);
	/*! @brief Dumps raw data directly into the FastCGI protocol
	 *
	 * This function exists as a mechanism to dump raw data out the stream bypassing
	 * the stream buffer.
	 *
	 * @param[in] data Pointer to first byte of data to send
	 * @param[in] size Size in bytes of data to be sent
	 */
	void dump(const char* data, size_t size);
	/*! @brief Dumps a byte string directy into the FastCGI protocol
	 *
	 * This function exists as a mechanism to dump raw data, in the form of a
	 * %string out the stream bypassing the stream buffer.
	 * 
	 * @param[in] str Const-reference to string to print
	 */
	void dump(u_string const& str);
	/*! @brief Dumps a byte string directy into the FastCGI protocol
	 *
	 * This function exists as a mechanism to dump raw data, in the form of a
	 * %string out the stream bypassing the stream buffer.
	 * 
	 * @param[in] str Const-reference to string to print
	 */
	void dump(std::string const& str);
	/*! @brief Dumps an input stream directly into the FastCGI protocol
	 *
	 * This function exists as a mechanism to dump a raw input stream out this stream bypassing
	 * the stream buffer. Typically this would be a filestream
	 * associated with an image or something. The stream is transmitted until an EOF.
	 *
	 * @param[in] stream Reference to input stream that should be transmitted.
	 */
	void dump(std::basic_istream<char>& stream);
	//@}
	/*! @brief Dumps an input stream directly into the FastCGI protocol
	 *
	 * This function exists as a mechanism to dump a raw input stream out this stream bypassing
	 * the stream buffer. Typically this would be a filestream
	 * associated with an image or something. The stream is transmitted until an EOF.
	 *
	 * @param[in] stream Reference to input stream that should be transmitted.
	 */
	void dump(std::basic_istream<uchar>& stream);
	//@}

private:
	/*! @brief Stream buffer class for output of client data through FastCGI
	 * This class is derived from std::basic_streambuf<_char_type, traits>. It acts just
	 * the same as any stream buffer does with the added feature of the dump() function.
	 */
	class Fcgibuf;
	//! Stream buffer object
	std::unique_ptr<Fcgibuf> pbuf;
};
/*! @name Buffered output operators */
//@{
/*! @brief Print a string to a Fcgistream
 *  @param os The stream to print to
 *  @param[in] s The string to print
 *  @return os
 */
Fcgistream& operator << (Fcgistream& os, std::string const& s);
/*! @brief Print a string to a Fcgistream
 *  @param os The stream to print to
 *  @param[in] us The string to print
 *  @return os
 */
Fcgistream& operator << (Fcgistream& os, u_string const& us);
/*! @brief Print a string to a Fcgistream
 *
 *  This function converts the wide string to a UTF-8 byte stream, then prints
 *  it.
 *  @param os The stream to print to
 *  @param[in] ws The string to print
 *  @return os
 */
Fcgistream& operator << (Fcgistream& os, std::wstring const& ws);
/*! @brief Print a string to a Fcgistream
 *
 *  @param os The stream to print to
 *  @param[in] s The string to print
 *  @return os
 */
Fcgistream& operator << (Fcgistream& os, const char* s);
/*! @brief Print a string to a Fcgistream
 *
 *  @param os The stream to print to
 *  @param[in] us The string to print
 *  @return os
 */
Fcgistream& operator << (Fcgistream& os, const uchar* us);
/*! @brief Print a string to a Fcgistream
 *
 *  This function converts the wide string to a UTF-8 byte stream, then prints
 *  it.
 *  @param os The stream to print to
 *  @param[in] ws The string to print
 *  @return os
 */
Fcgistream& operator << (Fcgistream& os, const wchar_t* ws);
//@}

MOSH_FCGI_END

#endif
