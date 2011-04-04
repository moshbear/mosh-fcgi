//! \file vec_ops.hpp - Wrapper for vector types; unifies stl, stxxl, etc.
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*                                                                          *
* This file is part of fastcgi++.                                          *
*                                                                          *
* fastcgi++ is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/
#ifndef VEC_OPS
#define VEC_OPS

VEC_OPS__BEGIN

template <typename vT>
struct vec_ops {
	template <typename InIt>
	static void append(vT&, InIt, InIt);
	template <typename InIt>
	static void assign(vT&, InIt, InIt);
};

#define VEC_OPS__GROUP(...) __VA_ARGS__
#define VEC_OPS__SPEC(t_spec, t_args, __append__, __assign__, ...) \
	template <t_spec> struct vec_ops<t_args> { \
		__VA_ARGS__ \
		template <typename InIt> \
		static void append(t_args& v, InIt s, InIt e) { \
			__append__; \
		} \
		template <typename InIt> \
		static void assign(t_args& v, InIt s, InIt e) { \
			__assign__; \
		} \
	}

#if (defined(VEC_OPS__STD) || (!defined(VEC_OPS__NO_STD)))
#include <vector>
VEC_OPS__SPEC(VEC_OPS__GROUP(typename T1, typename T2), 
		VEC_OPS__GROUP(std::vector<T1, T2>),
		v.insert(v.end(), s, e),
		v.assign(s, e)
);
#endif

#ifdef VEC_OPS__STXXL
#include <algorithm>
#include <boost/bind.hpp>
#include <stxxl/vector>
VEC_OPS__SPEC(VEC_OPS__GROUP(typename T1, unsigned T2, typename T3,
				unsigned T4, typename T5, typename T6),
		VEC_OPS__GROUP(stxxl::vector<T1, T2, T3, T4, T5, T6>),
		std::for_each(s, e, boost::bind(__pushback__, v, _1)),
		v.set_content(s, e, std::distance(s, e)),
		VEC_OPS__GROUP(VEC_OPS__GROUP(template <typename T1, unsigned T2, typename T3,
							unsigned T4, typename T5, typename T6>),
				void __pushback__(
					VEC_OPS__GROUP(stxxl::vector<T1, T2, T3,
								T4, T5 ,T6>& v),
					const T1& t) 
				{
					v.push_back(t); 
				})
);
#endif

#ifdef VEC_OPS__HV
#include <hybrid_vector.h>
VEC_OPS__SPEC(VEC_OPS__GROUP(typename T1, typename T2, typename T3),
		VEC_OPS__GROUP(hybrid_vector<T1, T2, T3>),
		v.append(s, e),
		v.assign(s, e)
);
#endif

VEC_OPS__END

#endif // VEC_OPS
