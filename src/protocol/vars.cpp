//! @file protocol/vars.cpp Defitions of extern constants in protocol::
//
/***************************************************************************
* Copyright (C) 2012 m0shbear                                              *
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

#include <mosh/fcgi/protocol/vars.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace protocol {

const char* record_type_labels[] = {
	"INVALID",
	"BEGIN_REQUEST",
	"ABORT_REQUEST",
	"END_REQUEST",
	"PARAMS",
	"IN",
	"OUT",
	"ERR",
	"DATA",
	"GET_VALUES",
	"GET_VALUES_RESULT",
	"UNKNOWN_TYPE"
};

}

MOSH_FCGI_END
