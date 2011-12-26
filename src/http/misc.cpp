//! @file http/misc.cpp - HTTP miscellany
/*!*************************************************************************
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

#include <algorithm>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <string>
#include <stdexcept>
#include <iomanip>

extern "C" {
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
}

#include <mosh/fcgi/http/misc.hpp>
#include <mosh/fcgi/bits/singleton.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {

// string cache
// functions as contextually last-used buffer
struct tts_lu : public MOSH_FCGI::Singleton<tts_lu> {
	
	// strftime %a, %A
	int dayofweek;
	std::shared_ptr<std::string> a_dayofweek;
	std::shared_ptr<std::string> f_dayofweek;
	// strftime %b, %B
	int month;
	std::shared_ptr<std::string> a_month;
	std::shared_ptr<std::string> f_month;
};

}

MOSH_FCGI_BEGIN

namespace http {

std::string time_to_string(const std::string& fmt, int add_seconds) {
	struct timeval tv;
	gettimeofday(&tv, 0);
	tv.tv_sec += add_seconds;
	struct tm tm;
	gmtime_r(&tv.tv_sec, &tm);	
	std::stringstream ss;
	bool hit_pct;
	char strft_buf[64];
	tts_lu& lu = tts_lu::instance();

	for (const char& ch : fmt) {
		switch (ch) {

#define _print_(len, fill, x) std::setw(len) << std::setfill(fill) << x

		case '%':
			if (hit_pct) {
				ss << '%'; 
				hit_pct = false;
			} else
				hit_pct = true;
			break;
		case 'a':
			if (hit_pct) {
				if ((tm.tm_wday != lu.dayofweek) || !lu.a_dayofweek) {
					strftime(strft_buf, 64, "%a", &tm);
					lu.a_dayofweek.reset(new std::string(strft_buf));
					lu.dayofweek = tm.tm_wday;
				}
				ss << *lu.a_dayofweek;
				hit_pct = false;
			} else
				ss << ch;
			break;
		case 'A':
			if (hit_pct) {
				if ((tm.tm_wday != lu.dayofweek) || !lu.f_dayofweek) {
					strftime(strft_buf, 64, "%A", &tm);
					lu.f_dayofweek.reset(new std::string(strft_buf));
					lu.dayofweek = tm.tm_wday;
				}
				ss << *lu.f_dayofweek;
				hit_pct = false;
			} else
				ss << ch;
			break;
		case 'b':
			if (hit_pct) {
				if ((tm.tm_mon != lu.month) || !lu.a_month) {
					strftime(strft_buf, 64, "%b", &tm);
					lu.a_month.reset(new std::string(strft_buf));
					lu.month = tm.tm_mon;
				}
				ss << *lu.a_month;
				hit_pct = false;
			} else
				ss << ch;
			break;
		case 'B':
			if (hit_pct) {
				if ((tm.tm_mon != lu.month) || !lu.f_month) {
					strftime(strft_buf, 64, "%B", &tm);
					lu.f_month.reset(new std::string(strft_buf));
					lu.month = tm.tm_mon;
				}
				ss << *lu.f_month;
				hit_pct = false;
			} else
				ss << ch;
			break;
		case 'd':
			if (hit_pct) {
				ss << _print_(2, '0', tm.tm_mday);
				hit_pct = false;
			} else
				ss << ch;
			break;
		case 'f':
			if (hit_pct) {
				ss << _print_(6, '0', tv.tv_usec);
				hit_pct = false;
			} else
				ss << ch;
			break;
		case 'H':
			if (hit_pct) {
				ss << _print_(2, '0', tm.tm_hour);
				hit_pct = false;
			} else
				ss << ch;
			break;
		case 'm':
			if (hit_pct) {
				ss << _print_(2, '0', tm.tm_mon + 1);
				hit_pct = false;
			} else
				ss << ch;
			break;
		case 'M':
			if (hit_pct) {
				ss << _print_(2, '0', tm.tm_min);
				hit_pct = false;
			} else
				ss << ch;
			break;
		case 'S':
			if (hit_pct) {
				ss << _print_(2, '0', tm.tm_sec);
				hit_pct = false;
			} else
				ss << ch;
			break;
		case 'Y':
			if (hit_pct) {
				ss << _print_(2, '0', tm.tm_year + 1900);
				hit_pct = false;
			} else
				ss << ch;
			break;
#undef _print_
		default:
			ss << ch;
		};
	}
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

