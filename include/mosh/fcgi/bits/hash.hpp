//! @file mosh/fcgi/bits/hash.hpp Hasher
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
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

#ifndef MOSH_FCGI_BITS_HASH_HPP
#define MOSH_FCGI_BITS_HASH_HPP

#include <utility>
#include <map>
#include <string>
extern "C" {
#include <nspr/prtypes.h>
#include <nss/hasht.h>
#include <nss/sechash.h>
}
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace hash {

template <typename T>
void hash(HASHContext* h, const std::basic_string<T>&s) {
	HASH_Update(h, reinterpret_cast<const unsigned char*>(s.data()), s.size() * sizeof(T));
}

void hash(HASHContext* h, const void* p, size_t len) {
	HASH_Update(h, reinterpret_cast<const unsigned char*>(p), len);
}

template <typename T1, typename T2>
void hash(HASHContext* h, const std::map<std::basic_string<T1>, std::basic_string<T2>>& m) {
	for (const auto& it : m) {
		hash(h, it.first);
		hash(h, it.second);
	}
}

std::vector<unsigned char> hash_finalize(HASHContext* h) {
	std::vector<unsigned char> v(HASH_ResultLenContext(h));
	unsigned vlen;
	HASH_End(h, v.data(), &vlen, HASH_ResultLenContext(h));
	v.resize(vlen);
	return v;
}
	
template <typename T>
std::vector<unsigned char> hash(const T& t) {
	HASHContext* c = HASH_Create(HASH_AlgSHA1);
	hash(c, t);
	auto v = hash_finalize(c);
	HASH_Destroy(c);
	return v;
}


}

MOSH_FCGI_END

#endif
