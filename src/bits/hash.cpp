//! @file hasher.cpp Hasher
/***************************************************************************
* Copyright (C) 2011-2 m0shbear                                            *
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

#include <vector>
extern "C" {
#include "sha/sha1.h"
}

#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/hash.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

Hash::Hash() {
	handle = sha1_new();
}

Hash::~Hash() {
	sha1_destroy(static_cast<sha1_state*>(handle));
}

void Hash::update(const uchar* data, size_t len) {
	sha1_process(static_cast<sha1_state*>(handle), data, len);
}

std::vector<uchar> Hash::finalize() {
	std::vector<uchar> v(20);
	sha1_done(static_cast<sha1_state*>(handle), v.data());
	return v;
}

void hash_update(Hash& h, const void* p, size_t len) {
	h.update(reinterpret_cast<const uchar*>(p), len);
}

MOSH_FCGI_END
