//! @file  mosh/fcgi/bits/hash.hpp Hasher
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
#include <vector>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! A generic hasher class
class Hash {
	void* handle;
public:
	//! Hash initiale
	Hash();
	//! Hash destroy
	virtual ~Hash();
	//! Hash update
	void update(const uchar* data, size_t len);
	//! Hash finalize
	std::vector<uchar> finalize();
};
	
template <typename T>
void hash_update(Hash& h, const std::basic_string<T>&s) {
	h.update(reinterpret_cast<const uchar*>(s.data()), s.size() * sizeof(T));
}

void hash_update(Hash& h, const void* p, size_t len);

template <typename T1, typename T2>
void hash_update(Hash& h, const std::map<std::basic_string<T1>, std::basic_string<T2>>& m) {
	for (const auto& it : m) {
		hash_update(h, it.first);
		hash_update(h, it.second);
	}
}

template <typename T>
std::vector<uchar> hash(const T& t) {
	Hash c;
	hash_update(c, t);
	auto v = c.finalize();
	return v;
}



MOSH_FCGI_END

#endif
