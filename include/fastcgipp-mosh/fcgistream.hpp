//! \file fcgistream.hpp Defines the Fastcgipp_m0sh::Fcgistream stream and stream buffer classes
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


#include <streambuf>
#include <ostream>
#include <cstring>
#include <algorithm>
#include <ios>
#include <istream>
#include <locale>


#include <fastcgipp-mosh/protocol.hpp>
#include <fastcgipp-mosh/transceiver.hpp>
#include <fastcgipp-mosh/bits/utf8_codecvt.hpp>

//! Topmost namespace for the fastcgi++ library
namespace Fastcgipp_m0sh
{

template<class charT> inline std::locale makeLocale(std::locale& loc)
{
        return loc;
}

template<> std::locale inline makeLocale<wchar_t>(std::locale& loc)
{
        return std::locale(loc, new utf8CodeCvt::utf8_codecvt_facet);
}

//! Stream class for output of client data through FastCGI
/*!
 * This class is derived from std::basic_ostream<charT, traits>. It acts just
 * the same as any stream does with the added feature of the dump() function.
 *
 * @tparam charT Character type (char or wchar_t)
 * @tparam traits Character traits
 */
template <class charT, class traits>
class Fcgistream: public std::basic_ostream<charT, traits>
{
public:
	Fcgistream() : std::basic_ostream<charT, traits>(&buffer) { }
	//! Arguments passed directly to Fcgibuf::set()
	void set(Protocol::FullId id_, Transceiver& transceiver_, Protocol::RecordType type_) {
		buffer.set(id_, transceiver_, type_);
	}

	//! Dumps raw data directly into the FastCGI protocol
	/*!
	 * This function exists as a mechanism to dump raw data out the stream bypassing
	 * the stream buffer or any code conversion mechanisms. If the user has any binary
	 * data to send, this is the function to do it with.
	 *
	 * @param[in] data Pointer to first byte of data to send
	 * @param[in] size Size in bytes of data to be sent
	 */
	void dump(char* data, size_t size) {
		buffer.dump(data, size);
	}
	//! Dumps an input stream directly into the FastCGI protocol
	/*!
	 * This function exists as a mechanism to dump a raw input stream out this stream bypassing
	 * the stream buffer or any code conversion mechanisms. Typically this would be a filestream
	 * associated with an image or something. The stream is transmitted until an EOF.
	 *
	 * @param[in] stream Reference to input stream that should be transmitted.
	 */
	void dump(std::basic_istream<char>& stream);
private:
	//! Stream buffer class for output of client data through FastCGI
	/*!
	 * This class is derived from std::basic_streambuf<charT, traits>. It acts just
	 * the same as any stream buffer does with the added feature of the dump() function.
	 *
	 * @tparam charT Character type (char or wchar_t)
	 * @tparam traits Character traits
	 */
	class Fcgibuf: public std::basic_streambuf<charT, traits>
	{
	public:
		Fcgibuf()
			: dumpSize(0), dumpPtr(0) {
			setp(buffer, buffer + buffSize);
		}
		//! After construction constructor
		/*!
		 * Sets FastCGI related member data necessary for operation of the
		 * stream buffer.
		 *
		 * @param[in] id_ Complete ID associated with the request
		 * @param[in] transceiver_ Transceiver object to use for transmission
		 * @param[in] type_ Type of output stream (ERR or OUT)
		 */
		void set(Protocol::FullId id_, Transceiver& transceiver_, Protocol::RecordType type_) {
			id = id_;
			transceiver = &transceiver_;
			type = type_;
		}

		virtual ~Fcgibuf() {
			try {
				sync();
			} catch (...) { } }
		//! Dumps raw data directly into the FastCGI protocol
		/*!
		 * This function exists as a mechanism to dump raw data out the stream bypassing
		 * the stream buffer or any code conversion mechanisms. If the user has any binary
		 * data to send, this is the function to do it with.
		 *
		 * @param[in] data Pointer to first byte of data to send
		 * @param[in] size Size in bytes of data to be sent
		 */
		void dump(char* data, size_t size) {
			dumpPtr = data;
			dumpSize = size;
			sync();
		}

	private:
		typedef typename std::basic_streambuf<charT, traits>::int_type int_type;
		typedef typename std::basic_streambuf<charT, traits>::traits_type traits_type;
		typedef typename std::basic_streambuf<charT, traits>::char_type char_type;

		int_type overflow(int_type c = traits_type::eof());

		int sync() {
			return emptyBuffer();
		}

		std::streamsize xsputn(const char_type *s, std::streamsize n);

		//! Pointer to the data that needs to be transmitted upon flush
		char* dumpPtr;
		//! Size of the data pointed to be dumpPtr
		size_t dumpSize;

		//! Code converts, packages and transmits all data in the stream buffer along with the dump data
		int emptyBuffer();
		//! Transceiver object to use for transmissio
		Transceiver* transceiver;
		//! Size of the internal stream buffer
		static const int buffSize = 8192;
		//! The buffer
		char_type buffer[buffSize];
		//! Complete ID associated with the request
		Protocol::FullId id;

		//! Type of output stream (ERR or OUT)
		Protocol::RecordType type;
	};
	//! Stream buffer object
	Fcgibuf buffer;
};
}
