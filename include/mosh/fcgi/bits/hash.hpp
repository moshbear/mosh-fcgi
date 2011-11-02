//! @file mosh/fcgi/bits/hash.hpp Hasher
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
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

#ifndef MOSH_FCGI_BITS_HASH_HPP
#define MOSH_FCGI_BITS_HASH_HPP

#include <utility>
#include <map>
#include <string>
#include <crypto++/ripemd.h>
#include <crypto++/sha.h>
#include <crypto++/tiger.h>
#include <crypto++/whrlpool.h>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace hash {

template <typename Hash_type, typename T>
void hash(Hash_type& h, const std::basic_string<T>&s) {
	h.Update(reinterpret_cast<const unsigned char*>(s.data()), s.size() * sizeof(T));
}

template <typename Hash_type>
void hash(Hash_type& h, const void* p, size_t len) {
	h.Update(reinterpret_cast<const unsigned char*>(p), len);
}

template <typename Hash_type, typename T1, typename T2>
void hash(Hash_type& h, const std::map<std::basic_string<T1>, std::basic_string<T2>>& m) {
	for (const auto& it : m) {
		hash(h, it.first);
		hash(h, it.second);
	}
}

template <typename Hash_type>
std::vector<unsigned char> hash_finalize(Hash_type& h) {
	std::vector<unsigned char> v(h.DigestSize());
	h.Final(v.data());
	return v;
}
	
template <typename Hash_type, typename T>
std::vector<unsigned char> hash(const T& t) {
	Hash_type hash;
	hash(hash, t);
	return hash_finalize(hash);
}


}

MOSH_FCGI_END

#endif
