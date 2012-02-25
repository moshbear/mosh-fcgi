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
#include <nspr/prtypes.h>
#include <nss/hasht.h>
#include <nss/sechash.h>
}

#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/hash.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

Hash::Hash() {
	handle = HASH_Create(HASH_AlgSHA1);
}

Hash::~Hash() {
	HASH_Destroy(static_cast<HASHContext*>(handle));
}

void Hash::update(const uchar* data, size_t len) {
	HASH_Update(static_cast<HASHContext*>(handle), data, len);
}

std::vector<uchar> Hash::finalize() {
	std::vector<uchar> v(HASH_ResultLenContext(static_cast<HASHContext*>(handle)));
	unsigned vlen;
	HASH_End(static_cast<HASHContext*>(handle), v.data(), &vlen,
			HASH_ResultLenContext(static_cast<HASHContext*>(handle)));
	v.resize(vlen);
	return v;
}

void hash_update(Hash& h, const void* p, size_t len) {
	h.update(reinterpret_cast<const uchar*>(p), len);
}

MOSH_FCGI_END
