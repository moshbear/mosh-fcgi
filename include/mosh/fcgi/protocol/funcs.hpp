//! @file  mosh/fcgi/protocol/funcs.hpp Protocol-related functions
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
#include <string>
#include <utility>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace protocol {

	/*! @brief Extract a name-value pair from a FastCGI PARAMS record
	 *
	 * @param[in] data Pointer to the record body
	 * @param[in] data_size Length of the record body
	 * @param[out] result Reference to pair holding name and value
	 * @return The number of bytes consumed
	 * @retval -1 Record body too small
	 */
	ssize_t process_param_record(const uchar* data, size_t data_size, std::pair<std::string, std::string>& result);
}

MOSH_FCGI_END

#endif
