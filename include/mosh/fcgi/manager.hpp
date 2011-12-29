//! @file mosh/fcgi/manager.hpp Defines the fcgi::Manager class
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

#ifndef MOSH_FCGI_MANAGER_HPP
#define MOSH_FCGI_MANAGER_HPP

#include <mosh/fcgi/manager/manager.hpp>
#include <mosh/fcgi/manager/manager.tcc>

#endif
