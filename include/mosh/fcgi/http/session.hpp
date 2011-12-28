//! @file mosh/fcgi/http/session.hpp Meta-include for session/*
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

#ifndef MOSH_FCGI_HTTP__SESSION_HPP
#define MOSH_FCGI_HTTP__SESSION_HPP

// MOSH_FCGI::http::Session_base<char_type>
#include <mosh/fcgi/http/session/session_base.hpp>
#include <mosh/fcgi/http/session/session_base.tcc>

// MOSH_FCGI::http::Session<char_type, post_val_type>
#include <mosh/fcgi/http/session/session.hpp>
#include <mosh/fcgi/http/session/session.tcc>

// MOSH_FCGI::http::Fcgi_session<char_type, post_val_type, data_val_type>
#include <mosh/fcgi/http/session/fcgi_session.hpp>

#endif
