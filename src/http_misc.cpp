//! @file http_misc.cpp - HTTP miscellany
/*!*************************************************************************
* Copyright (C) 2011 m0shbear                                              *
*                                                                          *
* This file is part of fastcgi++.                                          *
*                                                                          *
* fastcgi++ is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.                                          *
*                                                                          *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     *
* License for more details.                                                *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public License *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.       *
****************************************************************************/

#include <algorithm>
#include <locale>
#include <sstream>
#include <string>


#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/time_facet.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

extern "C" {
	#include <unistd.h>
}

#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

std::string time_to_string(const std::string& fmt) {
	using namespace boost::posix_time;
	namespace dt = boost::date_time;
	using namespace boost::gregorian;
	ptime now(microsec_clock::universal_time());
	std::locale loc(std::locale::classic(), new dt::time_facet<ptime, char>(fmt.c_str())); 
	std::stringstream ss;
	ss.imbue(loc);
	ss << now;
	return ss.str();
}

std::string hostname() {
	/* Create a string of sufficient length to be guaranteed to hold the hostname.
	 * SUSv2 guarantees that length shall be at most 255 bytes
	 * and POSIX.1-2001 guarantees that length shall be at most HOST_NAME_MAX bytes.
	 * In either case, we set the string's length to the greater of the two and
	 * add 1 to make room for the null terminator.
	 */
	size_t host_max = std::max(HOST_NAME_MAX, 255);
	std::string ss(host_max + 1, 0);
	gethostname(&ss[0], host_max + 1);
	/* gethostname used null-termination to size the string, so in order to resize
	 * the string to the actual length of the hostname, we find the null terminator.
	 * However, the filename must be at most 255 characters in length, so we
	 * use the lesser of (strlen(ss.c_str()), 64) as the new size.
	 */
	ss.resize(std::min(static_cast<int>(ss.find('\0')), 64));
	return ss;
}

}

MOSH_FCGI_END

