//! @file  mosh/fcgi/http/session/fcgi_session.hpp FastCGI HTTP Session
/***************************************************************************
* Copyright (C) 2011 m0shbear						   *
*									   *
* This file is part of mosh-fcgi.					   *
*									   *
* mosh-fcgi is free software: you can redistribute it and/or modify it	   *
* under the terms of the GNU Lesser General Public License as  published   *
* by the Free Software Foundation, either version 3 of the License, or (at *
* your option) any later version.					   *
*									   *
* mosh-fcgi is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or	   *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public	   *
* License for more details.						   *
*									   *
* You should have received a copy of the GNU Lesser General Public License *
* along with mosh-fcgi.  If not, see <http://www.gnu.org/licenses/>.	   *
****************************************************************************/


#ifndef MOSH_FCGI_HTTP_SESSION_FCGI_SESSION_HPP
#define MOSH_FCGI_HTTP_SESSION_FCGI_SESSION_HPP

#include <algorithm>
#include <string>
#include <vector>
#include <mosh/fcgi/http/session/session.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

namespace http {

/*! @brief HTTP Environment
 * @tparam char_type Character type to use for strings
 * @tparam post_val_type Value type for POST entries
 * @tparam data_val_type Value type for data_buffer (only applicable when ROLE == ROLE_FILTER)
 */
template <typename char_type, typename post_val_type = std::basic_string<char>, typename data_val_type = std::vector<char>>
class Fcgi_session : public virtual Session <char_type, post_val_type>
{
public:
	//! Input from DATA stream
	data_val_type data_buffer;

	Fcgi_session() { }

	virtual ~Fcgi_session() { }
	
	/*! @brief Appends the contents of a DATA record to the DATA buffer
	 * @param[in] data Pointer to the first byte of data
	 * @param[in] size Size of data in bytes
	 */
	void fill_data(const char* data, size_t size) {
		std::copy(data, data + size, std::back_inserter(data_buffer));
	}
};

} // namespace http

MOSH_FCGI_END

#endif


