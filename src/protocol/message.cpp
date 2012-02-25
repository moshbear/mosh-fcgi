//! @file  protocol/message.hpp mosh-fcgi message
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

#include <mosh/fcgi/protocol/message.hpp>
#include <mosh/fcgi/bits/namespace.hpp>
#include <src/array_deleter.hpp>
#include <src/namespace.hpp>

MOSH_FCGI_BEGIN

namespace protocol {

Message::Message() : data(0, SRC::Array_deleter<uchar>()) { }

}


MOSH_FCGI_END
