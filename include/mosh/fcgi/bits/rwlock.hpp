#include <cstddef>
#include <system_error>
#include <mutex>
extern "C" {
#include <pthread.h>
}

/*! @brief A reader-writer lock
 *  This class encapsulates %pthread_rwlock_* with C++ semantics.
 *  It also adds the ability to upgrade a read lock to a write one.
 */
class Rw_lock {
	static inline void throw_system_error(int e) {
		throw std::system_error(std::error_code(e, std::generic_category()));
	}
public:
	//! Create a new reader-writer lock
	Rw_lock() {
		// Initialize rw lock
		{
			int ret = pthread_rwlock_init(&lock, NULL);
			if (ret != 0)
				throw_system_error(ret);
		}
	}
	//! Destroy an existing reader-writer lock
	virtual ~Rw_lock() {
		// Ignore error code because destructors shouldn't throw
		pthread_rwlock_destroy(&lock);
	}

	/*! @name Read locks
	 */
	//@{
	/* @brief Acquire a read lock
	 * @throws std::system_error if @c pthread_rwlock_rdlock's return value is non-zero
	 */
	void read_lock() {
		int ret = pthread_rwlock_rdlock(&lock);
		if (ret != 0)
			throw_system_error(ret);
	}
	/* @brief Try to acquire a read lock
	 * @retval @c true if @c pthread_rwlock_tryrdlock's return value is zero
	 * @retval @c false otherwise
	 */
	bool try_read_lock() noexcept {
		return !pthread_rwlock_tryrdlock(&lock);
	}
	//@}

	/*! @name Write locks
	 */
	//@{
	/* @brief Acquire a write lock
	 * @throws std::system_error if @c pthread_rwlock_wrlock's return value is non-zero
	 */
	void write_lock() {
		int ret = pthread_rwlock_wrlock(&lock);
		if (ret != 0)
			throw_system_error(ret);
	}
	/* @brief Try to acquire a write lock
	 * @retval @c true if @c pthread_rwlock_trywrlock's return value is zero
	 * @retval @c false otherwise
	 */
	bool try_write_lock() noexcept {
		return !pthread_rwlock_trywrlock(&lock);
	}
	//@}
	
	//! Release a read or write lock
	void unlock() noexcept {
		pthread_rwlock_unlock(&lock);
	}

	/* @brief Upgrade a read lock to a write lock
	 * @sa unlock
	 * @sa write_lock
	 */
	void upgrade_lock() {
		std::lock_guard<std::mutex> up_lk(up_lock);
		unlock();
		write_lock();
	}
	
	//! Get the native handle (i.e. a pointer to the internal pthread_rwlock)
	auto native_handle() noexcept -> pthread_rwlock_t* {
		return &lock;
	}
private:
	//! Reader-writer lock
	pthread_rwlock_t lock;
	//! Upgrade lock
	std::mutex up_lock;
};


