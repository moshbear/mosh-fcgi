//! @file  mosh/fcgi/protocol.hpp include protocol/*
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

#ifdef MOSH_FCGI_USE_CGI
#error "MOSH_FCGI_USE_CGI must be undefined in FastCGI (default) mode"
#endif


#ifndef MOSH_FCGI_PROTOCOL_HPP
#define MOSH_FCGI_PROTOCOL_HPP

// enums, forward declarations for protocol::* classes
#include <mosh/fcgi/protocol/types.hpp>
// protocol constants
#include <mosh/fcgi/protocol/vars.hpp>
// prototype for process_param_header
#include <mosh/fcgi/protocol/funcs.hpp>

// protocol::Full_id
#include <mosh/fcgi/protocol/full_id.hpp> 
// protocol::Header
#include <mosh/fcgi/protocol/header.hpp>
// protocol::Begin_request
#include <mosh/fcgi/protocol/begin_request.hpp>
// protocol::Unknown_type
#include <mosh/fcgi/protocol/unknown_type.hpp>
// protocol::End_request
#include <mosh/fcgi/protocol/end_request.hpp>
// protocol::Management_reply<int, int, int>
#include <mosh/fcgi/protocol/management_reply.hpp>
// protocol::Message
#include <mosh/fcgi/protocol/message.hpp>
// protocol::_(gs?|s)_*
#include <mosh/fcgi/protocol/gs.hpp>

#endif
