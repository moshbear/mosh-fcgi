//! @file  tempfile.cpp - Temporary file
/*!*************************************************************************
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

#include <string>
#include <stdexcept>
#include <ios>
#include <fstream>
extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
}

#include <mosh/fcgi/bits/u.hpp>
#include <mosh/fcgi/bits/tempfile.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

namespace {

bool isdir(std::string const& path) {
	struct stat sb;
	if (stat(path.c_str(), &sb) == -1) {
		int e = errno;
		switch (e) {
		case ENOENT:
		case ENOTDIR:
			return false;
		default:
			throw std::runtime_error(std::string("isdir: ") + strerror(errno));
		}
	}
	return S_ISDIR(sb.st_mode);
}


void mkdir(std::string const& path) {
	errno = 0;
	if (::mkdir(path.c_str(), 0700) == -1)
		throw std::runtime_error(std::string("mkdir: ") + strerror(errno));
}
	
	

void mkdir_p(std::string path) {
	if (isdir(path))
		return;
	while (path.find('/') != path.rfind('/')) {
		std::string p0 = path.substr(0, path.rfind('/'));
		if (isdir(p0)) {
			mkdir(path);
		}
		path = std::move(p0);
	}
	if (path.find('/') != std::string::npos) {
		mkdir(path);
	}
}

}

MOSH_FCGI_BEGIN

Tempfile::Tempfile(std::string const& path) {
	if (path.find('/') != std::string::npos)
		mkdir_p(path.substr(0, path.find('/')));
	fname = path;
	file.reset(new std::ofstream);
	file->exceptions(std::ios_base::failbit | std::ios_base::badbit);
	file->open(path, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
}

Tempfile::~Tempfile() {
	if (file && !is_permanent) {
		unlink(fname.c_str());
	}
}

void Tempfile::write(const uchar* str, size_t len) {
	if (!file)
		throw std::runtime_error("uninitialized file");
	file->write(sign_cast<const char*>(str), len);
}

MOSH_FCGI_END





