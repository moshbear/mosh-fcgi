//! @file protocol/funcs.hpp Protocol-related functions
//
/***************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*               2007 Eddie                                                 *
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


#ifndef MOSH_FCGI_PROTOCOL_FUNCS_HPP
#define MOSH_FCGI_PROTOCOL_FUNCS_HPP

#include <cstddef>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace protocol {

	/*! @brief Process the body of a FastCGI parameter record
	 *
	 * Takes the body of a FastCGI record of type parameter and parses  it. You end
	 * up with a pointer/size for both the name and value of the parameter.
	 *
	 * @param[in] data Pointer to the record body
	 * @param[in] data_size Length of the record body
	 * @param[out] name Reference to a pointer that will be pointed to the first byte of the parameter name
	 * @param[out] name_size Reference to a value to will be given the size in bytes of the parameter name
	 * @param[out] value Reference to a pointer that will be pointed to the first byte of the parameter value
	 * @param[out] value_size Reference to a value to will be given the size in bytes of the parameter value
	 */
	bool process_param_header(const char* data, size_t data_size,
				const char*& name, size_t& name_size,
				const char*& value, size_t& value_size);

}

MOSH_FCGI_END

#endif
