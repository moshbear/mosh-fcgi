//! @file  mosh/fcgi/bits/aligned.hpp Aligned types
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

#ifndef MOSH_FCGI_ALIGNED_HPP
#define MOSH_FCGI_ALIGNED_HPP

#include <type_traits>
#include <cstring>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @name Aligned class template
 */
//@{
/*! @class aligned_as
 *  @brief Aligned class template
 *
 *  If in doubt about alignment and want to be sure that access won't cause an unaligned access,
 *  use this class to create a temporary aligned copy. UA is avoided by using %memcpy for the
 *  bitwise copy.
 *  @tparam AsT align-as type (must be POD)
 *  @tparam T value type (must be POD)
 */
	template <typename AsT, typename T>
	class
	alignas(alignof(AsT))
	aligned_as { 
	protected:
		typename std::enable_if<std::is_pod<AsT>::value, T>::type x;
	public: 
		//! construct defaultly
		aligned_as() : x(T())  { } 
		//! construct from reference 
		aligned_as(T const& t) { 
			std::memcpy(static_cast<void*>(&x), static_cast<const void*>(&t), sizeof(T)); 
		} 
		//! construct from pointer
		aligned_as(T const* const tp) { 
			std::memcpy(static_cast<void*>(&x), static_cast<const void*>(tp), sizeof(T)); 
		} 
		//! construct from void pointer
		aligned_as(void const* const vp) { 
			std::memcpy(static_cast<void*>(&x), vp, sizeof(T)); 
		} 
		virtual ~aligned_as() { } 
		//! assign from reference 
		aligned_as<AsT,T>& operator = (T const& t) {
			std::memcpy(static_cast<void*>(&x), static_cast<const void*>(&t), sizeof(T)); 
			return *this;
		} 
		//! assign from pointer
		aligned_as<AsT,T>& operator = (T const* const tp) { 
			std::memcpy(static_cast<void*>(&x), static_cast<const void*>(tp), sizeof(T)); 
			return *this;
		} 
		//! assign from void pointer
		aligned_as<AsT,T>& operator = (void const* const vp) { 
			std::memcpy(static_cast<void*>(&x), vp, sizeof(T)); 
			return *this;
		} 
		//! cast to reference
		operator T& () { 
			return x; 
		} 
		//! cast to add-const reference
		operator T const& () const { 
			return x; 
		} 
		//! cast to pointer
		operator T* () {
			return &x;
		}
		//! cast to const pointer
		operator T const* () const {
			return &x;
		}

	};

/*! @brief Aligned class template
 *
 *  If in doubt about alignment and want to be sure that access won't cause an unaligned access,
 *  use this class to create a temporary aligned copy. UA is avoided by using %memcpy for the
 *  bitwise copy.
 *  @tparam N alignment
 *  @tparam T value type (must be POD)
 */
	template <size_t N, typename T> 
	class
	alignas(N)
	aligned { 
	protected:
		T x;
	public: 
		//! construct defaultly
		aligned() : x(T()) { }
		//! construct from reference 
		aligned(T const& t) { 
			std::memcpy(static_cast<void*>(&x), static_cast<const void*>(&t), sizeof(T)); 
		} 
		//! construct from pointer
		aligned(T const* const tp) { 
			std::memcpy(static_cast<void*>(&x), static_cast<const void*>(tp), sizeof(T)); 
		} 
		//! construct from void pointer
		aligned(void const* const vp) { 
			std::memcpy(static_cast<void*>(&x), vp, sizeof(T)); 
		} 
		virtual ~aligned() { } 
		//! assign from reference 
		aligned<N,T>& operator = (T const& t) {
			std::memcpy(static_cast<void*>(&x), static_cast<const void*>(&t), sizeof(T)); 
			return *this;
		} 
		//! assign from pointer
		aligned<N,T>& operator = (T const* const tp) { 
			std::memcpy(static_cast<void*>(&x), static_cast<const void*>(tp), sizeof(T)); 
			return *this;
		} 
		//! assign from void pointer
		aligned<N,T>& operator = (void const* const vp) { 
			std::memcpy(static_cast<void*>(&x), vp, sizeof(T)); 
			return *this;
		} 
		//! cast to reference
		operator T& () { 
			return x; 
		} 
		//! cast to add-const reference
		operator T const& () const { 
			return x; 
		} 
	} ;
//@}
/* @name Zero-filled aligned class template
 */
//@{
/*! @brief Zero-fill aligned class template
 *
 *  If in doubt about alignment and want to be sure that access won't cause an unaligned access,
 *  use this class to create a temporary aligned copy. UA is avoided by using %memcpy for the
 *  bitwise copy.
 *  @tparam N alignment
 *  @tparam T value type (must be POD)
 *  @tparam T2 source value type (must be POD)
 */
template <size_t N, typename T, typename T2>
struct zerofill_aligned : public aligned<N, typename std::enable_if<std::is_pod<T2>::value, T>::type> {
	//! construct defaultly
	zerofill_aligned() { }
	//! construct from reference
	zerofill_aligned(T2 const& t) {
		aligned<N, T2> t_a(t);
		this->x = t_a;
	}
	//! construct from pointer
	zerofill_aligned(T2 const* const pt) {
		aligned<N, T2> t_a(*pt);
		this->x = t_a;
	}
	//! construct from void pointer
	zerofill_aligned(void const* const pv) {
		aligned<N, T2> t_a(*static_cast<T2 const* const>(pv));
		this->x = t_a;
	}
	virtual ~zerofill_aligned() { }
	//! assign from reference
	zerofill_aligned<N,T,T2>& operator = (T2 const& t) {
		aligned<N, T2> t_a(t);
		this->x = t_a;
		return *this;
	}
	//! assign from pointer
	zerofill_aligned<N,T,T2>& operator = (T2 const* const pt) {
		aligned<N, T2> t_a(*pt);
		this->x = t_a;
		return *this;
	}
	//! assign from void pointer
	zerofill_aligned<N,T,T2>& operator = (void const* const pv) {
		aligned<N, T2> t_a(*static_cast<T2 const* const>(pv));
		this->x = t_a;
		return *this;
	}
};

/*! @brief Zero-fill aligned class template
 *
 *  If in doubt about alignment and want to be sure that access won't cause an unaligned access,
 *  use this class to create a temporary aligned copy. UA is avoided by using %memcpy for the
 *  bitwise copy.
 *  @tparam AsT align-as type
 *  @tparam T value type
 *  @tparam T2 source value type
 */
template <typename AsT, typename T, typename T2>
struct zerofill_aligned_as : public aligned_as<AsT, typename std::enable_if<std::is_pod<T2>::value, T>::type> {
	//! construct defaultly
	zerofill_aligned_as() { }
	//! construct from reference
	zerofill_aligned_as(T2 const& t) {
		aligned_as<AsT, T2> t_a(t);
		this->x = t_a;
	}
	//! construct from pointer
	zerofill_aligned_as(T2 const* const pt) {
		aligned_as<AsT, T2> t_a(*pt);
		this->x = t_a;
	}
	//! construct from void pointer
	zerofill_aligned_as(void const* const pv) {
		aligned_as<AsT, T2> t_a(*static_cast<T2 const* const>(pv));
		this->x = t_a;
	}
	virtual ~zerofill_aligned_as() { }
	//! assign from reference
	zerofill_aligned_as<AsT,T,T2>& operator = (T2 const& t) {
		aligned_as<AsT, T2> t_a(t);
		this->x = t_a;
		return *this;
	}
	//! assign from pointer
	zerofill_aligned_as<AsT,T,T2>& operator = (T2 const* const pt) {
		aligned_as<AsT, T2> t_a(*pt);
		this->x = t_a;
		return *this;
	}
	//! assign from void pointer
	zerofill_aligned_as<AsT,T,T2>& operator = (void const* const pv) {
		aligned_as<AsT, T2> t_a(*static_cast<T2 const* const>(pv));
		this->x = t_a;
		return *this;
	}
};
//@}

MOSH_FCGI_END

#endif
