//! @file mosh/fcgi/fcgistream.hpp Defines the Fcgistream stream and stream buffer classes
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

#ifndef MOSH_FCGI_FCGISTREAM_FCGISTREAM_HPP
#define MOSH_FCGI_FCGISTREAM_FCGISTREAM_HPP

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
#include <mosh/fcgi/bits/iconv.hpp>
#include <mosh/fcgi/bits/native_utf.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! Stream class for output of client data through FastCGI
/*!
 * This class is derived from std::basic_ostream<_char_type, traits>. It acts just
 * the same as any stream does with the added feature of the dump() function.
 *
 * @tparam _char_type Character type (char or wchar_t)
 * @tparam traits Character traits
 */
template <class _char_type, class traits>
class Fcgistream: public std::basic_ostream<_char_type, traits> {
public:
	Fcgistream() : std::basic_ostream<_char_type, traits>(&buffer) { }
	//! Arguments passed directly to Fcgibuf::set()
	void set(protocol::Full_id id, Transceiver& transceiver, protocol::Record_type type) {
		buffer.set(id, transceiver, type);
	}
	/*! @name Dumpers
	 */
	//@{
	/*! @brief Dumps raw data directly into the FastCGI protocol
	 * This function exists as a mechanism to dump raw data out the stream bypassing
	 * the stream buffer or any code conversion mechanisms. If the user has any binary
	 * data to send, this is the function to do it with.
	 *
	 * @param[in] data Pointer to first byte of data to send
	 * @param[in] size Size in bytes of data to be sent
	 */
	void dump(const char* data, size_t size) {
		buffer.dump(data, size);
	}
	/*! @brief Dumps a byte string directy into the FastCGI protocol
	 * This function exists as a mechanism to dump raw data, in the form of a
	 * % strinf out the stream bypassing the stream buffer or any code conversion mechanisms.
	 * If the user has any binary data to send, this is the function to do it with.
	 * Typically, this is used for HTTP headers, which must be encoded in ASCII.
	 * 
	 * @param[in] str Const-reference to string to print
	 */
	void dump(std::string const& str) {
		buffer.dump(str.data(), str.size());
	}
	/*! @brief Dumps an input stream directly into the FastCGI protocol
	 * This function exists as a mechanism to dump a raw input stream out this stream bypassing
	 * the stream buffer or any code conversion mechanisms. Typically this would be a filestream
	 * associated with an image or something. The stream is transmitted until an EOF.
	 *
	 * @param[in] stream Reference to input stream that should be transmitted.
	 */
	void dump(std::basic_istream<char>& stream);
	//@}
	/*! @brief Sets the output character set.
	 * @throws std::invalid_argument if native encoding cannot be converted to argument type
	 * @param[in] s Output charset
	 */
	void set_output_charset(const std::string& s = "") {
		if (s.empty()) {
			if (sizeof(_char_type) > 1)
				buffer.ic.reset(Iconv::make_state("UTF-8", native_utf<sizeof(_char_type)>::value()));
			else
				buffer.ic.reset(Iconv::make_state("US-ASCII", native_utf<sizeof(_char_type)>::value())); 
		} else
			buffer.ic.reset(Iconv::make_state(s, native_utf<sizeof(_char_type)>::value()));
	} 

private:
	/*! @brief Stream buffer class for output of client data through FastCGI
	 * This class is derived from std::basic_streambuf<_char_type, traits>. It acts just
	 * the same as any stream buffer does with the added feature of the dump() function.
	 *
	 * @tparam _char_type Character type (char or wchar_t)
	 * @tparam traits Character traits
	 */
	class Fcgibuf: public std::basic_streambuf<_char_type, traits>
	{
	public:
		Fcgibuf() : dump_ptr(0), dump_size(0), ic(Iconv::make_state("", ""), Iconv::Deleter()) {
			setp(buffer, buffer + buff_size);
		}
		/*! @brief After construction constructor
		 * Sets FastCGI related member data necessary for operation of the
		 * stream buffer.
		 *
		 * @param[in] id Complete ID associated with the request
		 * @param[in] transceiver Transceiver object to use for transmission
		 * @param[in] type Type of output stream (Er_r or Ou_t)
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
		void dump(const char* data, size_t size) {
			dump_ptr = data;
			dump_size = size;
			sync();
		}

	private:
		typedef typename std::basic_streambuf<_char_type, traits>::int_type int_type;
		typedef typename std::basic_streambuf<_char_type, traits>::traits_type traits_type;
		typedef typename std::basic_streambuf<_char_type, traits>::char_type char_type;
		
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

		std::streamsize xsputn(const _char_type *s, std::streamsize n);

		//! Pointer to the data that needs to be transmitted upon flush
		const char* dump_ptr;
		//! Size of the data pointed to be dump_ptr
		size_t dump_size;

		//! Code converts, packages and transmits all data in the stream buffer along with the dump data
		int empty_buffer();
		//! Transceiver object to use for transmission
		Transceiver* transceiver;
		//! Size of the internal stream buffer
		static const int buff_size = 8192;
		//! The buffer
		_char_type buffer[buff_size];
		//! Complete ID associated with the request
		protocol::Full_id id;

		//! Type of output stream (Err or Out)
		protocol::Record_type type;

		std::shared_ptr<Iconv::IC_state> ic;
	};
	//! Stream buffer object
	Fcgibuf buffer;
};

MOSH_FCGI_END

#endif
