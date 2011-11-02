//! @file mosh/fcgi/bits/singleton.hpp Singleton<T> declaration
// Copyright © 2011 m0shbear <andrey@moshbear.net>
// Distributed under the Boost Software License, Version 1.0. (See accompany-
// ing file LICENSE10.txt or copy at http://www.boost.org/LICENSE10.txt)
//
#ifndef MOSH_FCGI_SINGLETON_HPP
#define MOSH_FCGI_SINGLETON_HPP

#include <boost/utility.hpp>
#include <boost/thread/once.hpp>
#include <boost/scoped_ptr.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @brief A singleton class.
 * A singleton of type @c T has only one instance running at once.
 * This allows for a run-time static class member initialization effect.
 * @tparam T class type
 */
template<class T>
class Singleton : private boost::noncopyable {
public:
	/*! @brief Get a reference to the instance
	 * If the instance hasn't been created, it is run.
	 * @return T&
	 */
	static T& instance() {
		boost::call_once(init, flag);
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
	 static boost::scoped_ptr<T> t;
	 static boost::once_flag flag;

};

MOSH_FCGI_END

// Initialize at compile-time to sane values
template<class T> boost::scoped_ptr<T> MOSH_FCGI::Singleton<T>::t(0);
template<class T> boost::once_flag MOSH_FCGI::Singleton<T>::flag = BOOST_ONCE_INIT;


#endif
