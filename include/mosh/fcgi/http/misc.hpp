//! @file mosh/fcgi/http/misc.hpp Non-templated miscellany for http
/*!*************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*                                                                          *
* This file is part of mosh-cgi.                                           *
*                                                                          *
* This library is free software: you can redistribute it and/or modify it  *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* This library is distributed in the hope that it will be useful, but      *
* WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser *
* General Public License for more details.                                 *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with mosh-cgi.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/


#ifndef MOSH_FCGI_HTTP_MISC_HPP
#define MOSH_FCGI_HTTP_MISC_HPP

#include <string>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN


namespace http {

/*! @brief Prints the current UTC time, in microsecond resolution to a string, with format fmt
 *  This implements the following features:
 *  <table>
 *  	<tr><td>%a</td><td>Abbreviated weekday name</td></tr>
 *  	<tr><td>%A</td><td>Full weekday name</td></tr>
 *  	<tr><td>%b</td><td>Abbreviated month name</td></tr>
 *  	<tr><td>%B</td><td>Full month name</td></tr>
 *  	<tr><td>%d</td><td>Day of month (01-31)</td></tr>
 *  	<tr><td>%f</td><td>Fractional seconds</td></tr>
 *  	<tr><td>%H</td><td>Hour of day (00-23)</td></tr>
 *  	<tr><td>%m</td><td>Month (01-12)</td></tr>
 *  	<tr><td>%M</td><td>Minute (00-60)</td></tr>
 *  	<tr><td>%S</td><td>Second (00-60)</td></tr>
 *  	<tr><td>%Y</td><td>Year (0000-9999)</td></tr>
 *  	<tr><td>%</td><td>Literal %</td></tr>
 *  </table>
 *
 *  While not 100% complete, it does everything I need it to do.
 *
 *  @param[in] fmt format string
 *  @param[in] add_seconds time offset, in seconds
 */
std::string time_to_string(const std::string& fmt, int add_seconds = 0);

//! Gets the current hostname
std::string hostname();


}

MOSH_FCGI_END

#endif
