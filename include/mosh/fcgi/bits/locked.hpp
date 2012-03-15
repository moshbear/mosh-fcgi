/*! @file  mosh/fcgi/bits/locked.hpp Lock-augmented types
 *
 * Type wrapper templates which orthogonally augment locks. That is,
 * @c Mutexed&lt;T&gt; is @c T with mutex members. Accessing @c T's members does not
 * lock, nor does locking introduce side effects to accessing @c T's members.
 * The same logic applies to @c Rw_locked&lt;T&gt;
 *
 * For non-class types, use the @c _0 form, which acts as a composite of mutex
 * and @c T, except that there is no inheritance from @c T.
 */
/***************************************************************************
* Copyright (C) 2012 m0shbear                                              *
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

#ifndef MOSH_FCGI_LOCKED_HPP
#define MOSH_FCGI_LOCKED_HPP

#include <mutex>
#include <utility>
#include <mosh/fcgi/bits/rwlock.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @brief Mutex type wrapper
 *
 *  @warning does not lock when members of T are accessed
 *
 *  @tparam T type to wrap
 */
template <typename T>
struct Mutexed : public T, public std::mutex {
public:
	/*! @name Constructors
	 */
	//@{
	Mutexed() : T() { }
	Mutexed(T const& t) : T(t) { }
	Mutexed(T && t) : T(std::move(t)) { }
	//@}

	virtual ~Mutexed() { }
};

/*! @brief Mutex type wrapper for non-class types
 *
 *  For unions and primitive types, @c : @c public @c T doesn't work, so
 *  @c T is embedded into the structure and @c static_cast&lt;T @c cv&&gt; is required
 *  if union members are accessed.
 *
 *  @note Relational operators are undefined since it's impossible to make them both generic and well-formed
 *  @note
 *  @note As are rvalue-reference operators, but here it's more the fact that move semantics aren't needed
 *
 *  @tparam T type to wrap
 */
template <typename T>
class Mutexed_0 : public std::mutex {
public:
	/*! @name Constructors
	 */
	//@{
	Mutexed_0() : _t() { }
	Mutexed_0(T const& t) : _t(t) { }
	//@}
	virtual ~Mutexed_0() { }
	
	operator T () const { return _t; }
	operator T& () { return _t; }
	operator T const& () const { return _t; }

	Mutexed_0<T>& operator = (T const& t) { _t = t; return *this; }
	Mutexed_0<T>& operator = (Mutexed_0<T> const& m) { _t = m._t; return *this; }
	// no operator = ((T|_m) &&) - unnecessary for primitives and unions
private:
	T _t;
};

/*! @brief Rw_lock type wrapper
 *
 *  @tparam T type to wrap
 */
template <typename T>
struct Rw_locked : public T, public Rw_lock {
public:
	/*! @name Constructors
	 */
	//@{
	Rw_locked() : T() { }
	Rw_locked(T const& t) : T(t) { }
	Rw_locked(T && t) : T(std::move(t)) { }
	//@}
	virtual ~Rw_locked() { }
};
	

MOSH_FCGI_END

#endif
