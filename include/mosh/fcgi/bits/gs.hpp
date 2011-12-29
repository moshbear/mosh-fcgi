//! @file bits/gs.hpp Getter-setters
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


#ifndef MOSH_FCGI_GS_HPP
#define MOSH_FCGI_GS_HPP

extern "C" {
#include <netinet/in.h> // for ntohs, ntohl
#include <arpa/inet.h>  //
}
#include <cstdint>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @name Getter-setters
 */
//@{
//! Defines a getter-setter
#define MOSH_FCGI_GETTERSETTER_T(EX_T, IN_T, GET, SET, ALIAS) \
	class _gs_##ALIAS { \
		IN_T* _x; \
		_gs_##ALIAS () = delete; \
	public: \
		_gs_##ALIAS (IN_T& x) : _x(&x) { } \
		/* Getter */ \
		operator EX_T () const { \
			return GET(*_x); \
		} \
		/* Setter */ \
		void operator = (EX_T && x) { \
			*_x = SET(x); \
		} \
	}

//! Defines a getter
#define MOSH_FCGI_GETTER_T(EX_T, IN_T, GET, ALIAS) \
	class _g_##ALIAS { \
		IN_T const* _x; \
		_g_##ALIAS () = delete; \
		void operator = (EX_T &&) = delete; \
	public: \
		_g_##ALIAS (IN_T const& x) : _x(&x) { } \
		/* Getter */ \
		operator EX_T () const { \
			return GET(*_x); \
		} \
	}

//! Defines a setter
#define MOSH_FCGI_SETTER_T(EX_T, IN_T, SET, ALIAS) \
	class _s_##ALIAS { \
		IN_T* _x; \
		_s_##ALIAS () = delete; \
		operator EX_T () const = delete; \
	public: \
		_s_##ALIAS (IN_T& x) : _x(&x) { } \
		/* Setter */ \
		void operator = (EX_T && x) { \
			*_x = SET(x); \
		} \
	}

//! Defines a getter-setter, getter, and setter
#define MOSH_FCGI_GETSET_T(EX_T, IN_T, GET, SET, ALIAS) \
	MOSH_FCGI_GETTERSETTER_T(EX_T, IN_T, GET, SET, ALIAS); \
	MOSH_FCGI_GETTER_T(EX_T, IN_T, GET, ALIAS); \
	MOSH_FCGI_SETTER_T(EX_T, IN_T, SET, ALIAS)
//@}
/*! @name Getter-setters for smart pointers (ones using get()/reset() semantics)
 */
//@{
//! Defines a getter-setter for smart pointers (ones using get()/reset() semantics)
#define MOSH_FCGI_GETTERSETTER_SMARTPTR_T(EX_T, IN_T, GET, SET, ALIAS) \
	class _gs_smartptr_##ALIAS { \
		IN_T* _x; \
		_gs_smartptr_##ALIAS () = delete; \
	public: \
		_gs_smartptr_##ALIAS (IN_T& x) : _x(&x) { } \
		/* Getter */ \
		operator EX_T () const { \
			return GET(_x->get()); \
		} \
		/* Setter */ \
		void operator = (EX_T && x) { \
			_x->reset(SET(x)); \
		} \
	}

//! Defines a getter for smart pointers (ones using get() semantics)
#define MOSH_FCGI_GETTER_SMARTPTR_T(EX_T, IN_T, GET, ALIAS) \
	class _g_smartptr_##ALIAS { \
		IN_T const* _x; \
		_g_smartptr_##ALIAS () = delete; \
		void operator = (EX_T &&) = delete; \
	public: \
		_g_smartptr_##ALIAS (IN_T const& x) : _x(&x) { } \
		/* Getter */ \
		operator EX_T () const { \
			return GET(_x->get()); \
		} \
	}

//! Defines a setter for smart pointers (ones using reset() semantics)
#define MOSH_FCGI_SETTER_SMARTPTR_T(EX_T, IN_T, SET, ALIAS) \
	class _s_smartptr_##ALIAS { \
		IN_T* _x; \
		_s_smartptr_##ALIAS () = delete; \
		operator EX_T () const = delete; \
	public: \
		_s_smartptr_##ALIAS (IN_T& x) : _x(&x) { } \
		/* Setter */ \
		void operator = (EX_T && x) { \
			_x->reset(SET(x)); \
		} \
	}

//! Defines a getter-setter, getter, and setter
#define MOSH_FCGI_GETSET_SMARTPTR_T(EX_T, IN_T, GET, SET, ALIAS) \
	MOSH_FCGI_GETTERSETTER_SMARTPTR_T(EX_T, IN_T, GET, SET, ALIAS); \
	MOSH_FCGI_GETTER_SMARTPTR_T(EX_T, IN_T, GET, ALIAS); \
	MOSH_FCGI_SETTER_SMARTPTR_T(EX_T, IN_T, SET, ALIAS)
//@}
MOSH_FCGI_END

#endif


