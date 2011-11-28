//! @file mosh/fcgi/fcgistream/fcgistream.tcc Defines member functions for Fcgistream
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*               2007 Eddie                                                 *
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

#ifndef MOSH_FCGI_FCGISTREAM_FCGISTREAM_TCC
#define MOSH_FCGI_FCGISTREAM_FCGISTREAM_TCC

#include <algorithm>
#include <locale>
#include <limits>
#include <cstdint>
#include <cstring>
#include <ios>
#include <mosh/fcgi/exceptions.hpp>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/protocol/full_id.hpp>
#include <mosh/fcgi/protocol/header.hpp>
#include <mosh/fcgi/fcgistream.hpp>
#include <mosh/fcgi/bits/iconv.hpp>
#include <mosh/fcgi/bits/iconv_cvt.hpp>
#include <mosh/fcgi/bits/iconv.tcc>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

template <class char_type, class traits>
int Fcgistream<char_type, traits>::Fcgibuf::empty_buffer() {
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
		if (wanted_size > numeric_limits<uint16_t>::max()) wanted_size = numeric_limits<uint16_t>::max();
		Block data_block(transceiver->request_write(wanted_size));
		data_block.size = (data_block.size / chunk_size) * chunk_size;
		char* to_next = data_block.data + sizeof(Header);
		locale loc = this->getloc();
		if (count) {
			if (sizeof(char_type) != sizeof(char)) {
				if (use_facet<codecvt<char_type, char, Iconv::IC_state*> >(loc)
				        .out(ic.get(), p_stream_pos, this->pptr(), p_stream_pos,
				             to_next, data_block.data + data_block.size, to_next)
				        == codecvt_base::error) {
					pbump(-(this->pptr() - this->pbase()));
					dump_size = 0;
					dump_ptr = 0;
					throw exceptions::Stream(id);
				}
			} else {
				size_t cnt = min(data_block.size - sizeof(Header), count);
				memcpy(data_block.data + sizeof(Header), p_stream_pos, cnt);
				p_stream_pos += cnt;
				to_next += cnt;
			}
		}
		size_t dumped_size = min(dump_size, static_cast<size_t>(data_block.data + data_block.size - to_next));
		memcpy(to_next, dump_ptr, dumped_size);
		dump_ptr += dumped_size;
		dump_size -= dumped_size;
		uint16_t content_length = to_next - data_block.data + dumped_size - sizeof(Header);
		uint8_t content_remainder = content_length % chunk_size;
		Header& header = *(Header*)data_block.data;
		header.set_version(version);
		header.set_type(type);
		header.set_request_id(id.fcgi_id);
		header.set_content_length(content_length);
		header.set_padding_length(content_remainder ? (chunk_size - content_remainder) : content_remainder);
			transceiver->secure_write(sizeof(Header) + content_length + header.get_padding_length(), id, false);
	}
	pbump(-(this->pptr() - this->pbase()));
	return 0;
}

template <class _char_type, class traits>
std::streamsize Fcgistream<_char_type, traits>::Fcgibuf::xsputn(const _char_type *s, std::streamsize n)
{
	std::streamsize remainder = n;
	while (remainder) {
		std::streamsize actual = std::min(remainder, this->epptr() - this->pptr());
		std::memcpy(this->pptr(), s, actual * sizeof(_char_type));
		this->pbump(actual);
		remainder -= actual;
		if (remainder) {
			s += actual;
			empty_buffer();
		}
	}

	return n;
}

template<class char_type, class traits > void Fcgistream<char_type, traits>::dump(std::basic_istream<char>& stream)
{
	const size_t buffer_size = 32768;
	char buffer[buffer_size];

	while (stream.good()) {
		stream.read(buffer, buffer_size);
		dump(buffer, stream.gcount());
	}
}

MOSH_FCGI_END

#endif
