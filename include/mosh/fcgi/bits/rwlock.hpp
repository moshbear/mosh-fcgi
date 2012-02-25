//! @file  mosh/fcgi/bits/rwlock.hpp Readers-writer lock
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

#ifndef MOSH_FCGI_RWLOCK_HPP
#define MOSH_FCGI_RWLOCK_HPP

#include <cstddef>
#include <system_error>
#include <mutex>
extern "C" {
#include <pthread.h>
}
#include <mosh/fcgi/bits/namespace.hpp>

#ifndef PTHREAD_ONCE_INIT
#error "Non-pthreads implementation does not exist"
#endif

MOSH_FCGI_BEGIN

/*! @brief A reader-writer lock
 *
 *  This class encapsulates %pthread_rwlock_* with C++ semantics.
 *  It also adds the ability to upgrade a read lock to a write one.
 *
 *  This also implements %Lockable, so it can be placed inside std::lock_guard<T>.
 *  @note For %Lockable mode, lock() only produces a read lock - write locking
 *  @note always requires use of upgrade_lock().
 */
class Rw_lock {
public:
	//! Create a new reader-writer lock
	Rw_lock();
	//! Destroy an existing reader-writer lock
	virtual ~Rw_lock();

	/*! @name Read locks
	 */
	//@{
	/*! @brief Acquire a read lock
	 * @throws std::system_error if @c pthread_rwlock_rdlock's return value is non-zero
	 */
	void read_lock();
	/*! @brief Acquire a read lock
	 * @see read_lock
	 */
	void lock();
	/*! @brief Try to acquire a read lock
	 * @retval @c true if @c pthread_rwlock_tryrdlock's return value is zero
	 * @retval @c false otherwise
	 */
	bool try_read_lock() noexcept;
	/*! @brief Try to acquire a read lock
	 * @see try_read_lock
	 */
	bool try_lock() noexcept;
	//@}

	/*! @name Write locks
	 */
	//@{
	/*! @brief Acquire a write lock
	 * @throws std::system_error if @c pthread_rwlock_wrlock's return value is non-zero
	 */
	void write_lock();
	/*! @brief Try to acquire a write lock
	 * @retval @c true if @c pthread_rwlock_trywrlock's return value is zero
	 * @retval @c false otherwise
	 */
	bool try_write_lock() noexcept;
	//@}
	
	//! Release a read or write lock
	void unlock() noexcept;

	/*! @brief Upgrade a read lock to a write lock
	 * @sa unlock
	 * @sa write_lock
	 */
	void upgrade_lock();
	
	//! Get the native handle (i.e. a pointer to the internal pthread_rwlock)
	auto native_handle() noexcept -> pthread_rwlock_t* {
		return &lck;
	}
private:
	//! Reader-writer lock
	pthread_rwlock_t lck;
	//! Upgrade lock
	std::mutex up_lock;
};

MOSH_FCGI_END

#endif
