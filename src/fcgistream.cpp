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
#include <mosh/fcgi/bits/block.hpp>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/protocol/header.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/fcgistream.hpp>
#include <mosh/fcgi/bits/utf8.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

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
		char* to_next = data_block.data + sizeof(Header);
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
		Header& header = *(Header*)data_block.data;
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

void Fcgistream::dump(std::basic_istream<char>& stream)
{
	std::array<char, 32768> buffer;

	while (stream.good()) {
		stream.read(buffer, buffer.size());
		dump(buffer, stream.gcount());
	}
}

void Fcgistream::dump(std::basic_istream<uchar>& stream)
{
	std::array<uchar, 32768> buffer;

	while (stream.good()) {
		stream.read(buffer, buffer.size());
		dump(buffer, stream.gcount());
	}
}

Fcgistream& operator << (Fcgistream& os, std::wstring const& ws) {
	const wchar_t* w_begin = ws.begin();
	const wchar_t* w_end = ws.end();
	std::array<uchar, 32768> buffer;
	char *s_next;
	while (w_begin != w_end) {
		if (utf8_out(w_begin, w_end, w_begin, buffer, buffer + 32768, s_next) < 0)
			throw std::invalid_argument("Fcgistream::operator<<(Fcgistream&, std::wstring const&): EILSEQ");
		os.write(buffer, std::distance(buffer, s_next);
	}
	return os;
}

MOSH_FCGI_END

