//! @file mosh/fcgi/bits/singleton.hpp Singleton<T> declaration
// Copyright © 2011 m0shbear <andrey@moshbear.net>
// Distributed under the Boost Software License, Version 1.0. (See accompany-
// ing file LICENSE10.txt or copy at http://www.boost.org/LICENSE10.txt)
//
#ifndef MOSH_FCGI_SINGLETON_HPP
#define MOSH_FCGI_SINGLETON_HPP

#include <mutex>
#include <memory>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

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

MOSH_FCGI_END

// Initialize at compile-time to sane values
template<class T> std::unique_ptr<T> MOSH_FCGI::Singleton<T>::t(nullptr);
template<class T> std::once_flag MOSH_FCGI::Singleton<T>::flag;


#endif
