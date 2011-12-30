//! @file  mosh/fcgi/fcgistream/fcgistream.hpp Defines the Fcgistream stream and stream buffer classes
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
#include <cstring>
#include <cwchar>

#ifndef MOSH_FCGI_USE_CGI
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/transceiver.hpp>
#endif
#include <mosh/fcgi/bits/iconv.hpp>
#include <mosh/fcgi/bits/iconv_gs.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

template <typename _char_type> class Fcgistream;
Fcgistream<_char_type>& operator << (Fcgistream<_char_type>&, std::basic_string<wchar_t> const&);
Fcgistream<_char_type>& operator << (Fcgistream<_char_type>&, wchar_t);
Fcgistream<_char_type>& operator << (Fcgistream<_char_type>&, wchar_t*);

//! Stream class for output of client data through FastCGI
/*!
 * This class is derived from std::ostream with the following modification:
 * 
 * operator<< (fcgistream&, std::basic_string<wchar_t> const&) is added.
 *
 * With this in place, both unicode and byte input can be buffered without
 * having to resort to the dump() hack.
 */
template <typename _char_type>
class Fcgistream: public std::ostream {
	friend Fcgistream<_char_type>& operator << (Fcgistream<_char_type>&, std::basic_string<wchar_t> const&);
	friend Fcgistream<_char_type>& operator << (Fcgistream<_char_type>&, wchar_t);
	friend Fcgistream<_char_type>& operator << (Fcgistream<_char_type>&, wchar_t*);
public:
#ifndef MOSH_FCGI_USE_CGI
	Fcgistream() : std::ostream(&buffer) { }
	//! Arguments passed directly to Fcgibuf::set()
	void set(protocol::Full_id id, Transceiver& transceiver, protocol::Record_type type) {
		buffer.set(id, transceiver, type);
	}
#else
	//! Arguments passed directly to Fcgibuf::set()
	Fcgistream(int _fd) : std::ostream(&buffer), buffer(_fd) { }
#endif
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
	/*! @brief Get a setter for the charset
	 * @throws std::invalid_argument if native encoding cannot be converted to argument type
	 */
	Iconv::_s_smartptr_s_ic_state<_char_type> output_charset() {
		return Iconv::_s_smartptr_s_ic_state<_char_type>(buffer.ic);
	}
	
private:
	/*! @brief Stream buffer class for output of client data through FastCGI
	 * This class is derived from std::basic_streambuf<_char_type, traits>. It acts just
	 * the same as any stream buffer does with the added feature of the dump() function.
	 *
	 * @tparam _char_type Character type (char or wchar_t)
	 * @tparam traits Character traits
	 */
	class Fcgibuf: public std::streambuf
	{
	public:
#ifndef MOSH_FCGI_USE_CGI
		Fcgibuf() : ic(Iconv::make_state("", ""), Iconv::Deleter()), dump_ptr(0), dump_size(0) {
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
#else
		Fcgibuf(int _fd) : ic(Iconv::make_state("", ""), Iconv::Deleter()), dump_ptr(0), dump_size(0), fd(_fd) {
			setp(buffer, buffer + buff_size);
		}
#endif

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

		std::shared_ptr<Iconv::IC_state> ic;

	private:
		typedef std::streambuf::int_type int_type;
		typedef std::streambuf::traits_type traits_type;
		typedef std::streambuf::char_type char_type;
		
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

		std::streamsize xsputn(const char *s, std::streamsize n);

		//! Pointer to the data that needs to be transmitted upon flush
		const char* dump_ptr;
		//! Size of the data pointed to be dump_ptr
		size_t dump_size;

		//! Code converts, packages and transmits all data in the stream buffer along with the dump data
		int empty_buffer();
#ifndef MOSH_FCGI_USE_CGI

		//! Transceiver object to use for transmission
		Transceiver* transceiver;
		//! Complete ID associated with the request
		protocol::Full_id id;
		//! Type of output stream (err or out)
		protocol::Record_type type;
#else
		//! This stream's fd
		int fd;
#endif
		//! Size of the internal stream buffer
		static const int buff_size = 8192;
		//! The buffer
		char buffer[buff_size];
	};
	//! Stream buffer object
	Fcgibuf buffer;
};

Fcgistream<_char_type>& operator << (Fcgistream<_char_type>& fs, std::basic_string<wchar_t> const wc&) {
	Iconv::IC_state* ic = fs.buffer.ic.get();
			
		
	
Fcgistream<_char_type>& operator << (Fcgistream<_char_type>& fs, wchar_t wc);

MOSH_FCGI_END

#endif
