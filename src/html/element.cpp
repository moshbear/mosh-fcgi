//! @file html/element.cpp Non-templated functions in MOSH_FCGI::html::Element
/* 
 *  Copyright (C) 2012 m0shbear
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 */

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <mosh/fcgi/html/element.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {

using namespace MOSH_FCGI::html::element::type_check;

const std::map<Type, std::set<Feature>> check =
{
	{ Type::unary, { Feature::attribute }},
	{ Type::binary, { Feature::attribute, Feature::data }},
	{ Type::comment, { Feature::data }},
	{ Type::begin, { Feature::attribute }},
	{ Type::end, std::set<Feature>() }
};

}

MOSH_FCGI_BEGIN

namespace html {

namespace element {

namespace type_check {

void checker<true>::value(Type t, Feature f) {
	if (!checker<false>::value(t, f))
		throw std::invalid_argument("type check fail");
}

bool checker<false>::value(Type t, Feature f) {
	const auto t_it = check.find(t);
	if (t_it == check.end())
		return false;
	const auto f_it = t_it->second.find(f);
	return f_it != t_it->second.end();
}
	
}

}

}

MOSH_FCGI_END
