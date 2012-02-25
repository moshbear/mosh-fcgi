//! @file  src/singleton.hpp Singleton<T> declaration
// Copyright © 2011-2 m0shbear <andrey@moshbear.net>
// Distributed under the Boost Software License, Version 1.0. (See accompany-
// ing file LICENSE10.txt or copy at http://www.boost.org/LICENSE10.txt)
//
#ifndef SRC_SINGLETON_HPP
#define SRC_SINGLETON_HPP

#include <mutex>
#include <memory>
#include <src/namespace.hpp>

SRC_BEGIN

/*! @brief A singleton class.
 *
 * A singleton of type @c T has only one instance running at once.
 * This allows for a run-time static class member initialization effect.
 * @tparam T class type
 */
template<class T>
class Singleton {
public:
	/*! @brief Get a reference to the instance
	 * If the instance hasn't been created, it is run.
	 * @return T&
	 */
	static T& instance() {
		std::call_once(flag, init);
		return *t;
	}
	//! Initialize the singleton's instance pointer
	static void init() throw () {
		t.reset(new T());
	}

protected:
	~Singleton() {}
	 Singleton() {}

private:
	static std::unique_ptr<T> t;
	static std::once_flag flag;
	
	Singleton(const Singleton<T>&) = delete; // NonCopyable
	Singleton(Singleton<T>&&) = delete; // NonMoveable
	const Singleton<T>& operator = (const Singleton<T>&) = delete;  // NonCopyAssignable
	const Singleton<T>& operator = (Singleton<T>&&) = delete; // NonMoveAssignable
};

SRC_END

// Initialize at compile-time to sane values
template<class T> std::unique_ptr<T> SRC::Singleton<T>::t(nullptr);
template<class T> std::once_flag SRC::Singleton<T>::flag;


#endif
