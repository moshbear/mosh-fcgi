//! @file  mosh/fcgi/protocol/vars.hpp Protocol constants and pseudo-constants
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


#ifndef MOSH_FCGI_PROTOCOL_VARS_HPP
#define MOSH_FCGI_PROTOCOL_VARS_HPP

#include <map>
#include <string>
#include <cstdint>
#include <mosh/fcgi/protocol/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace protocol {
	
	//! Defines text labels for the Record_type values
	extern const char* record_type_labels[];
	
	//! The version of the FastCGI protocol that this adheres to
	const uint8_t version = 1;
	//! All FastCGI records will be a multiple of this many bytes
	const size_t chunk_size = 8;
}

MOSH_FCGI_END

#endif

