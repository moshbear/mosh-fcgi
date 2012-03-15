//! @file fcgistream.cpp Defines member functions for Fcgistream
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

#include <array>
#include <algorithm>
#include <limits>
#include <locale>
#include <memory>
#include <cstdint>
#include <cstring>
#include <ios>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/block.hpp>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/protocol/header.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/fcgistream.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/utf8.hpp>
#include <src/namespace.hpp>

MOSH_FCGI_BEGIN

class Fcgistream::Fcgibuf: public std::basic_streambuf<uchar> {
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
		
		// Unbuffered send data
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
	static const size_t buff_size = 8192;
	//! The buffer
	uchar buffer[buff_size];
};

Fcgistream::Fcgistream() : std::basic_ostream<uchar>(), pbuf(new Fcgibuf) {
	this->rdbuf(pbuf.get());
}

Fcgistream::~Fcgistream() {
}

void Fcgistream::set(protocol::Full_id id, Transceiver& transceiver, protocol::Record_type type) {
	pbuf->set(id, transceiver, type);
}

void Fcgistream::dump(const uchar* data, size_t size) {
	pbuf->dump(data, size);
}

void Fcgistream::dump(const char* data, size_t size) {
	pbuf->dump(reinterpret_cast<const uchar*>(data), size);
}

void Fcgistream::dump(u_string const& str) {
	pbuf->dump(str.data(), str.size());
}

void Fcgistream::dump(std::string const& str) {
	pbuf->dump(reinterpret_cast<const uchar*>(str.data()), str.size());
}

void Fcgistream::dump(std::basic_istream<char>& stream) {
	std::array<char, 32768> buffer;

	while (stream.good()) {
		stream.read(buffer.data(), buffer.size());
		dump(sign_cast<uchar*>(buffer.data()), stream.gcount());
	}
}

void Fcgistream::dump(std::basic_istream<uchar>& stream) {
	std::array<uchar, 32768> buffer;

	while (stream.good()) {
		stream.read(buffer.data(), buffer.size());
		dump(buffer.data(), stream.gcount());
	}
}

Fcgistream& operator << (Fcgistream& os, std::string const& s) {
	os.rdbuf()->sputn(reinterpret_cast<const unsigned char *>(s.data()), s.size());
	return os;
}

Fcgistream& operator << (Fcgistream& os, u_string const& us) {
	os.rdbuf()->sputn(us.data(), us.size());
	return os;
}

Fcgistream& operator << (Fcgistream& os, std::wstring const& ws) {
	const wchar_t* w_begin = ws.data();
	const wchar_t* w_end = ws.data() + ws.size();
	std::array<uchar, 32768> buffer;
	uchar* s_next;
	while (w_begin != w_end) {
		if (SRC::utf8_out(w_begin, w_end, w_begin, buffer.data(), buffer.data() + 32768, s_next) < 0)
			throw std::invalid_argument("Fcgistream::operator<<(Fcgistream&, std::wstring const&): EILSEQ");
		os.rdbuf()->sputn(buffer.data(), std::distance(buffer.data(), s_next));
	}
	return os;
}

Fcgistream& operator << (Fcgistream& os, const char* s) {
	os.rdbuf()->sputn(sign_cast<const uchar*>(s), std::char_traits<char>::length(s));
	return os;
}

Fcgistream& operator << (Fcgistream& os, const uchar* s) {
	os.rdbuf()->sputn(s, std::char_traits<char>::length(sign_cast<const char*>(s)));
	return os;
}

Fcgistream& operator << (Fcgistream& os, const wchar_t* ws) {
	const wchar_t* w_begin = ws;
	const wchar_t* w_end = ws + std::char_traits<wchar_t>::length(ws);
	std::array<uchar, 32768> buffer;
	uchar* s_next;
	while (w_begin != w_end) {
		if (SRC::utf8_out(w_begin, w_end, w_begin, buffer.data(), buffer.data() + 32768, s_next) < 0)
			throw std::invalid_argument("Fcgistream::operator<<(Fcgistream&, std::wstring const&): EILSEQ");
		os.rdbuf()->sputn(buffer.data(), std::distance(buffer.data(), s_next));
	}
	return os;
}

int Fcgistream::Fcgibuf::empty_buffer() {
	using namespace std;
	using namespace protocol;
	char_type const* p_stream_pos = this->pbase();
	while (1) {
		size_t count = this->pptr() - p_stream_pos;
		size_t wanted_size = count * sizeof(char_type) + dump_size;
		if (!wanted_size)
			break;
		int remainder = wanted_size % chunk_size;
		wanted_size += sizeof(Header) + (remainder ? (chunk_size - remainder) : remainder);
		if (wanted_size > numeric_limits<uint16_t>::max())
			wanted_size = numeric_limits<uint16_t>::max();

		Block data_block(transceiver->request_write(wanted_size));
		data_block.size = (data_block.size / chunk_size) * chunk_size;
		locale loc = this->getloc();
		uchar* to_next = data_block.data + sizeof(Header);

		if (count) {
			size_t cnt = min(data_block.size - sizeof(Header), count);
			memcpy(data_block.data + sizeof(Header), p_stream_pos, cnt);
			p_stream_pos += cnt;
			to_next += cnt;
		}
		size_t dumped_size = min(dump_size, static_cast<size_t>(data_block.data + data_block.size - to_next));
		memcpy(to_next, dump_ptr, dumped_size);
		dump_ptr += dumped_size;
		dump_size -= dumped_size;
		uint16_t content_length = to_next - data_block.data + dumped_size - sizeof(Header);
		uint8_t content_remainder = content_length % chunk_size;
		Header& header = *reinterpret_cast<Header*>(data_block.data);
		header.version() = version;
		header.type() = type;
		header.request_id() = id.fcgi_id;
		header.content_length() = content_length;
		header.padding_length() = content_remainder ? (chunk_size - content_remainder) : content_remainder;
		transceiver->secure_write(sizeof(Header) + content_length + header.padding_length(), id, false);
	}
	pbump(-(this->pptr() - this->pbase()));
	return 0;
}

std::streamsize Fcgistream::Fcgibuf::xsputn(const uchar* s, std::streamsize n)
{
	std::streamsize remainder = n;
	while (remainder) {
		std::streamsize actual = std::min(remainder, this->epptr() - this->pptr());
		std::memcpy(this->pptr(), s, actual);
		this->pbump(actual);
		remainder -= actual;
		if (remainder) {
			s += actual;
			empty_buffer();
		}
	}

	return n;
}

MOSH_FCGI_END

