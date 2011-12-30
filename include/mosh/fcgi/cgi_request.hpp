//! @file  mosh/fcgi/cgi_request.hpp Defines the CGI (not FastCGI) request class
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

#ifndef MOSH_FCGI_USE_CGI
#error "CGI mode requires MOSH_FCGI_USE_CGI being defined"
#endif

#ifndef MOSH_FCGI_CGI_REQUEST_HPP
#define MOSH_FCGI_CGI_REQUEST_HPP

#include <locale>
#include <memory>
#include <stdexcept>
#include <string>

extern "C" {
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
}

#include <mosh/fcgi/bits/array_deleter.hpp>
#include <mosh/fcgi/bits/iconv.hpp>
#include <mosh/fcgi/bits/iconv_cvt.hpp>
#include <mosh/fcgi/http/session.hpp>
#include <mosh/fcgi/fcgistream.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

extern char** environ;

MOSH_FCGI_BEGIN

/*! @brief CGI Request handling class
 *
 * Derivations of this class will handle requests. This
 * includes building the session data, processing post/get data,
 * fetching data (files, database), and producing a response.
 * Once all client data is organized, response() will be called.
 * At minimum, derivations of this class must define response().
 *
 * If you want to use UTF-8 encoding pass wchar_t as the template
 * argument, use setloc() to setup a UTF-8 locale and use wide
 * character unicode internally for everything. If you want to use
 * a 8bit character set encoding pass char as the template argument and
 * setloc() a locale with the corresponding character set.
 *
 * @tparam char_type Character type for internal processing (wchar_t or char)
 * @tparam post_vec_type Vector type for storing file data
 */
template <typename char_type, typename post_val_type = std::basic_string<char_type>>
class Cgi_request {
public:
	//! Initialization.
	Cgi_request() : out(1), err(2) {
		setloc(std::locale::classic());
		out.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	}

	/*! @brief Response generator
	 *
	 * This function is called by handler() once all request data has been received from the other side.
	 * The function shall return true if it has completed the response and false otherwise (e.g., on error).
	 *
	 * @return Boolean value indication completion (true means complete)
	 * @sa callback
	 */
	virtual bool response() = 0;

	/*! @brief Request handler
	 *
	 * This function is called by the application (after application-specific startup, etc. )to
	 * perform a CGI request.
	 *
	 * @return return value of response()
	 * @sa response
	 */
	bool handler() {
		// Read env
		{
			char** e = environ;
			if (e == NULL)
				throw std::runtime_error("environ is NULL");
			while (*e != NULL) {
				session.fill(*e, strlen(*e));
				++e;
			}
		}
		// Read stdin
		{
			constexpr size_t rd_buf_size = 65535; // FastCGI uses u16be for content length, so we're limited
							      // to ((2^16)-1)
			std::unique_ptr<uchar, Array_deleter<uchar>> rd_buf(new uchar[rd_buf_size]);
			for (;;) {
				ssize_t did_read = read(0, rd_buf.get(), rd_buf_size);
				if (did_read == 0) {
					break;
				}
				else if (did_read < 0) {
					throw std::runtime_error(std::string("Error in Cgi_request::read: ") + strerror(errno));
				}
				session.fill_post(rd_buf.get(), did_read);
				in_handler(did_read);
			}
		}
		// Write response
		return response();
	}


protected:
	//! Structure containing all HTTP session data
	http::Session<char_type, post_val_type> session;

	/*! @brief Standard output stream to the client
	 *
	 * To dump data directly through the stream without it being code converted and bypassing
	 * the stream buffer call Fcgistream::dump()
	 */
	Fcgistream<char_type, std::char_traits<char_type>> out;

	/*! @brief Output stream to the HTTP server error log
	 *
	 * To dump data directly through the stream without it being code converted and bypassing
	 * the stream buffer call Fcgistream::dump()
	 */
	Fcgistream<char_type, std::char_traits<char_type>> err;

	/*! @brief Generate a data input response
	 *
	 * This function exists should the library user wish to do something like generate a partial response based on
	 * bytes received from the client. The function is called by handler() every time %read() returns a positive value.
	 * The function has no access to the data, but knows exactly how much was received based on the value that was passed.
	 * Note this value represents the amount of data received in the individual record, not the total amount received in
	 * the session. If the library user wishes to have such a value they would have to keep a tally of all size values
	 * passed.
	 *
	 * @param[in] bytes_received Amount of bytes received in this FastCGI record
	 */
	virtual bool in_handler(ssize_t bytes_received) { return true; }
	
	//! The locale associated with the request. Should be set with setloc(), not directly.
	std::locale loc;

	/*! @brief Set the requests locale
	 *
	 * This function both sets loc to the locale passed to it and imbues the locale into the
	 * out and err stream. The user should always call this function as opposed to setting the
	 * locales directly is this functions insures the iconv code conversion is functioning properly.
	 *
	 * @param[in] loc New locale
	 * @sa loc
	 * @sa out
	 */
	void setloc(std::locale loc) {
		loc = std::locale(loc, new Iconv_cvt<char_type, char>);
		out.imbue(loc);
		err.imbue(loc);
	}

};

MOSH_FCGI_END

#endif
