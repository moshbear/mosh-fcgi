//! @file  mosh/fcgi/bits/tempfile.hpp - Temporary file
/*!*************************************************************************
* Copyright (C) 201w m0shbear                                              *
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


#ifndef MOSH_FCGI_TEMPFILE_HPP
#define MOSH_FCGI_TEMPFILE_HPP

#include <fstream>
#include <memory>
#include <string>
#include <stdexcept>
#include <utility>

#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

/*! @brief An object representing a temporary file. 
 *  @warning Is not a subclass of std::ofstream, so iostream stuff
 *  @warning is unimplemented.
 */
class Tempfile {
public:
	/*! @brief Default constructor
	 *
	 * Member functions will throw std::runtime_error if called.
	 */
	Tempfile() {
	}
	/*! @brief Path constructor
	 *
	 * This constructor expects a file path and makes prerequisite
	 * directory/ies as needed.
	 */
	Tempfile(std::string const& path);
	//! Move constructor
	Tempfile(Tempfile&& f) {
		file = std::move(f.file);
		fname = std::move(f.fname);
		is_permanent = f.is_permanent;
	}
		
	virtual ~Tempfile();

	//! Move assignment
	Tempfile& operator = (Tempfile&& f) {
		if (this != &f) {
			file = std::move(f.file);
			fname = std::move(f.fname);
			is_permanent = f.is_permanent;
		}
		return *this;
	}

	/*! @brief Make this file permanent
	 *
	 * If this method is run, then the file represented by this object is
	 * not deleted on destruction.
	 */
	void make_permanent() {
		is_permanent = true;
	}

	//! Get the filename
	std::string const& filename() const {
		return fname;
	}

	//! Get the file's size
	size_t filesize() const {
		if (!file)
			throw std::runtime_error("uninitialized file");
		return file->tellp();
	}

	//! Write bytes to the file
	void write(const uchar* ptr, size_t count);

	//! Get the initialization state
	operator bool () const {
		return (!fname.empty()) && file;
	}

private:
	Tempfile(Tempfile const&) = delete;
	Tempfile& operator = (Tempfile const&) = delete;

	std::unique_ptr<std::ofstream> file;
	std::string fname;
	bool is_permanent;
};

MOSH_FCGI_END

#endif


