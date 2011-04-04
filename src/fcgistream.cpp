//! \file fcgistream.cpp Defines member functions for Fastcgipp_m0sh::Fcgistream
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


#include <fastcgipp-mosh/fcgistream.hpp>
#include <fastcgipp-mosh/protocol.hpp>
#include <fastcgipp-mosh/bits/utf8_cvt.hpp>

namespace Fastcgipp_m0sh
{

template int Fastcgipp_m0sh::Fcgistream<char, std::char_traits<char> >::Fcgibuf::emptyBuffer();
template int Fastcgipp_m0sh::Fcgistream<wchar_t, std::char_traits<wchar_t> >::Fcgibuf::emptyBuffer();
template <class charT, class traits>
int Fastcgipp_m0sh::Fcgistream<charT, traits>::Fcgibuf::emptyBuffer()
{
	using namespace std;
	using namespace Protocol;
	char_type const* pStreamPos = this->pbase();
	while (1) {{
			size_t count = this->pptr() - pStreamPos;
			size_t wantedSize = count * sizeof(char_type) + dumpSize;
			if (!wantedSize)
				break;

			int remainder = wantedSize % chunkSize;
			wantedSize += sizeof(Header) + (remainder ? (chunkSize - remainder) : remainder);
			if (wantedSize > numeric_limits<uint16_t>::max()) wantedSize = numeric_limits<uint16_t>::max();
			Block dataBlock(transceiver->requestWrite(wantedSize));
			dataBlock.size = (dataBlock.size / chunkSize) * chunkSize;

			mbstate_t cs = mbstate_t();
			char* toNext = dataBlock.data + sizeof(Header);

			locale loc = this->getloc();
			if (count) {
				if (sizeof(char_type) != sizeof(char)) {
					if (use_facet<codecvt<char_type, char, mbstate_t> >(loc)
					        .out(cs, pStreamPos, this->pptr(), pStreamPos,
					             toNext, dataBlock.data + dataBlock.size, toNext)
					        == codecvt_base::error) {
						pbump(-(this->pptr() - this->pbase()));
						dumpSize = 0;
						dumpPtr = 0;
						throw Exceptions::Stream(id);
					}
				} else {
					size_t cnt = min(dataBlock.size - sizeof(Header), count);
					memcpy(dataBlock.data + sizeof(Header), pStreamPos, cnt);
					pStreamPos += cnt;
					toNext += cnt;
				}
			}

			size_t dumpedSize = min(dumpSize, static_cast<size_t>(dataBlock.data + dataBlock.size - toNext));
			memcpy(toNext, dumpPtr, dumpedSize);
			dumpPtr += dumpedSize;
			dumpSize -= dumpedSize;
			uint16_t contentLength = toNext - dataBlock.data + dumpedSize - sizeof(Header);
			uint8_t contentRemainder = contentLength % chunkSize;

			Header& header = *(Header*)dataBlock.data;
			header.setVersion(version);
			header.setType(type);
			header.setRequestId(id.fcgiId);
			header.setContentLength(contentLength);
			header.setPaddingLength(contentRemainder ? (chunkSize - contentRemainder) : contentRemainder);

			transceiver->secureWrite(sizeof(Header) + contentLength + header.getPaddingLength(), id, false);
		}
	}
	pbump(-(this->pptr() - this->pbase()));
	return 0;
}

template std::streamsize Fastcgipp_m0sh::Fcgistream<char, std::char_traits<char> >::Fcgibuf::xsputn(const char_type *s, std::streamsize n);
template std::streamsize Fastcgipp_m0sh::Fcgistream<wchar_t, std::char_traits<wchar_t> >::Fcgibuf::xsputn(const char_type *s, std::streamsize n);
template <class charT, class traits>
std::streamsize Fastcgipp_m0sh::Fcgistream<charT, traits>::Fcgibuf::xsputn(const char_type *s, std::streamsize n)
{
	std::streamsize remainder = n;
	while (remainder) {
		std::streamsize actual = std::min(remainder, this->epptr() - this->pptr());
		std::memcpy(this->pptr(), s, actual * sizeof(char_type));
		this->pbump(actual);
		remainder -= actual;
		if (remainder) {
			s += actual;
			emptyBuffer();
		}
	}

	return n;
}

template Fastcgipp_m0sh::Fcgistream<char, std::char_traits<char> >::Fcgibuf::int_type Fastcgipp_m0sh::Fcgistream<char, std::char_traits<char> >::Fcgibuf::overflow(Fastcgipp_m0sh::Fcgistream<char, std::char_traits<char> >::Fcgibuf::int_type c = traits_type::eof());
template Fastcgipp_m0sh::Fcgistream<wchar_t, std::char_traits<wchar_t> >::Fcgibuf::int_type Fastcgipp_m0sh::Fcgistream<wchar_t, std::char_traits<wchar_t> >::Fcgibuf::overflow(Fastcgipp_m0sh::Fcgistream<wchar_t, std::char_traits<wchar_t> >::Fcgibuf::int_type c = traits_type::eof());
template <class charT, class traits>
typename Fastcgipp_m0sh::Fcgistream<charT, traits>::Fcgibuf::int_type Fastcgipp_m0sh::Fcgistream<charT, traits>::Fcgibuf::overflow(Fastcgipp_m0sh::Fcgistream<charT, traits>::Fcgibuf::int_type c)
{
	if (emptyBuffer() < 0)
		return traits_type::eof();
	if (!traits_type::eq_int_type(c, traits_type::eof()))
		return sputc(c);
	else
		return traits_type::not_eof(c);
}

template void Fastcgipp_m0sh::Fcgistream<char, std::char_traits<char> >::dump(std::basic_istream<char>& stream);
template void Fastcgipp_m0sh::Fcgistream<wchar_t, std::char_traits<wchar_t> >::dump(std::basic_istream<char>& stream);
template<class charT, class traits > void Fastcgipp_m0sh::Fcgistream<charT, traits>::dump(std::basic_istream<char>& stream)
{
	const size_t bufferSize = 32768;
	char buffer[bufferSize];

	while (stream.good()) {
		stream.read(buffer, bufferSize);
		dump(buffer, stream.gcount());
	}
}

}
