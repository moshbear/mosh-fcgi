//! @file  mosh/fcgi/fcgistream.hpp Defines the Fcgistream stream and stream buffer classes
/***************************************************************************
* Copyright (C) 2011-2 m0shbear                                              *
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
#include <streambuf>
#include <ostream>
#include <cstring>
#include <algorithm>
#include <ios>
#include <istream>
#include <locale>
#include <memory>

#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/transceiver.hpp>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! Stream class for output of client data through FastCGI
/*!
 * This class is derived from std::basic_ostream<unsigned char>. It acts just
 * the same as any stream does with the added feature of the dump() function.
 *
 */
class Fcgistream: public std::basic_ostream<uchar> {
public:
	Fcgistream() : std::basic_ostream<uchar>(&buffer) { }
	//! Arguments passed directly to Fcgibuf::set()
	void set(protocol::Full_id id, Transceiver& transceiver, protocol::Record_type type) {
		buffer.set(id, transceiver, type);
	}
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
	void dump(const uchar* data, size_t size) {
		buffer.dump(data, size);
	}
	/*! @brief Dumps raw data directly into the FastCGI protocol
	 *
	 * This function exists as a mechanism to dump raw data out the stream bypassing
	 * the stream buffer.
	 *
	 * @param[in] data Pointer to first byte of data to send
	 * @param[in] size Size in bytes of data to be sent
	 */
	void dump(const char* data, size_t size) {
		buffer.dump(reinterpret_cast<const uchar*>(data), size);
	}
	/*! @brief Dumps a byte string directy into the FastCGI protocol
	 *
	 * This function exists as a mechanism to dump raw data, in the form of a
	 * %string out the stream bypassing the stream buffer.
	 * 
	 * @param[in] str Const-reference to string to print
	 */
	void dump(u_string const& str) {
		buffer.dump(str.data(), str.size());
	}
	/*! @brief Dumps a byte string directy into the FastCGI protocol
	 *
	 * This function exists as a mechanism to dump raw data, in the form of a
	 * %string out the stream bypassing the stream buffer.
	 * 
	 * @param[in] str Const-reference to string to print
	 */
	void dump(std::string const& str) {
		buffer.dump(reinterpret_cast<const uchar*>(str.data()), str.size());
	}
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
	class Fcgibuf: public std::basic_streambuf<uchar>
	{
	public:
		Fcgibuf() : dump_ptr(0), dump_size(0) {
			setp(buffer, buffer + buff_size);
		}
		/*! @brief After construction constructor
		 * Sets FastCGI related member data necessary for operation of the
		 * stream buffer.
		 *
		 * @param[in] id Complete ID associated with the request
		 * @param[in] transceiver Transceiver object to use for transmission
		 * @param[in] type Type of output stream (err or out)
		 */
		void set(protocol::Full_id id, Transceiver& transceiver, protocol::Record_type type) {
			this->id = id;
			this->transceiver = &transceiver;
			this->type = type;
		}

		virtual ~Fcgibuf() {
			try {
				sync();
			} catch (...) { } }
		/*! @brief Dumps raw data directly into the FastCGI protocol
		 * This function exists as a mechanism to dump raw data out the stream bypassing
		 * the stream buffer or any code conversion mechanisms. If the user has any binary
		 * data to send, this is the function to do it with.
		 *
		 * @param[in] data Pointer to first byte of data to send
		 * @param[in] size Size in bytes of data to be sent
		 */
		void dump(const uchar* data, size_t size) {
			dump_ptr = data;
			dump_size = size;
			sync();
		}

	private:
		typedef typename std::basic_streambuf<uchar>::int_type int_type;
		typedef typename std::basic_streambuf<uchar>::traits_type traits_type;
		typedef typename std::basic_streambuf<uchar>::char_type char_type;
		
		int_type overflow(int_type c = traits_type::eof()) {
			if (empty_buffer() < 0)
				return traits_type::eof();
			if (!traits_type::eq_int_type(c, traits_type::eof()))
				return sputc(c);
			else
				return traits_type::not_eof(c);
		}

		int sync() {
			return empty_buffer();
		}

		std::streamsize xsputn(const uchar* s, std::streamsize n);

		//! Pointer to the data that needs to be transmitted upon flush
		const uchar* dump_ptr;
		//! Size of the data pointed to be dump_ptr
		size_t dump_size;

		//! Code converts, packages and transmits all data in the stream buffer along with the dump data
		int empty_buffer();

		//! Transceiver object to use for transmission
		Transceiver* transceiver;
		//! Complete ID associated with the request
		protocol::Full_id id;
		//! Type of output stream (err or out)
		protocol::Record_type type;
		//! Size of the internal stream buffer
		static const int buff_size = 8192;
		//! The buffer
		uchar buffer[buff_size];
	};
	//! Stream buffer object
	Fcgibuf buffer;
	
};

/*! @brief Print a string to a Fcgistream
 *  @param os The stream to print to
 *  @param[in] s The string to print
 *  @return os
 */
Fcgistream& operator << (Fcgistream& os, std::string const& s) {
	os.write(reinterpret_cast<const unsigned char *>(s.data()), s.size());
	return os;
}

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
 *  @param os The stream to print to
 *  @param[in] s The string to print
 *  @return os
 */
Fcgistream& operator << (Fcgistream& os, std::basic_string<uchar> const& us) {
	os.write(us.data(), us.size());
	return os;
}

MOSH_FCGI_END

#endif
