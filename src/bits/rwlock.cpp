// TODO: nt locking (look at VC11 <mutex> for hints)
//! @file  bits/rwlock.hpp Readers-writer lock
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

#include <cstddef>
#include <system_error>
#include <mutex>
extern "C" {
#include <pthread.h>
}
#include <mosh/fcgi/bits/rwlock.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

#ifndef PTHREAD_ONCE_INIT
#error "Non-pthreads implementation does not exist"
#endif

namespace {
	void throw_system_error(int e) {
		throw std::system_error(std::error_code(e, std::generic_category()));
	}
}

MOSH_FCGI_BEGIN

Rw_lock::Rw_lock() {
	// Initialize rw lock
	{
		int ret = pthread_rwlock_init(&lck, NULL);
		if (ret != 0)
			throw_system_error(ret);
	}
}

Rw_lock::~Rw_lock() {
	// Ignore error code because destructors shouldn't throw
	pthread_rwlock_destroy(&lck);
}

void Rw_lock::read_lock() {
	int ret = pthread_rwlock_rdlock(&lck);
	if (ret != 0)
		throw_system_error(ret);
}

void Rw_lock::lock() {
	read_lock();
} 

bool Rw_lock::try_read_lock() noexcept {
	return !pthread_rwlock_tryrdlock(&lck);
}
bool Rw_lock::try_lock() noexcept {
	return try_read_lock();
}

void Rw_lock::write_lock() {
	int ret = pthread_rwlock_wrlock(&lck);
	if (ret != 0)
		throw_system_error(ret);
}
bool Rw_lock::try_write_lock() noexcept {
	return !pthread_rwlock_trywrlock(&lck);
}

void Rw_lock::unlock() noexcept {
	pthread_rwlock_unlock(&lck);
}

void Rw_lock::upgrade_lock() {
	std::lock_guard<std::mutex> up_lk(up_lock);
	unlock();
	write_lock();
}
	
MOSH_FCGI_END

