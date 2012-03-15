//! @file src/protocol.hpp - src/ protocol functions
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


#ifndef SRC_PROTOCOL_HPP
#define SRC_PROTOCOL_HPP

#include <string>
#include <utility>
#include <src/u.hpp>
#include <src/namespace.hpp>

SRC_BEGIN

/*! @brief Convert a name-value pair to a PARAM record
 *
 * @param param The key-value pair to print
 */
u_string make_param_record(std::pair<std::string, std::string> const& param);

SRC_END

#endif
