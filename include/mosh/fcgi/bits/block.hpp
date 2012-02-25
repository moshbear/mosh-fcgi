//! @file  mosh/fcgi/bits/block.hpp  Defines the (transceiver's) Block class
/***************************************************************************
* Copyright (C) 2011-2 m0shbear                                            *
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


#ifndef MOSH_FCGI_BLOCK_HPP
#define MOSH_FCGI_BLOCK_HPP

#include <cstddef>
#include <mosh/fcgi/bits/types.hpp>
#include <mosh/fcgi/bits/namespace.hpp>

MOSH_FCGI_BEGIN

//! A raw block of memory
/*!
 * The purpose of this structure is to communicate a block of data to be written to
 * a Transceiver::Buffer
 */
struct Block {
	//! Construct from a pointer and size
	/*!
	 * @param[in] data Pointer to start of memory location
	 * @param[in] size Size in bytes of memory location
	 */
	Block(uchar* data, size_t size): data(data), size(size) { }
	//! Copies pointer and size, not data
	Block(const Block& block): data(block.data), size(block.size) { }
	Block(Block& block) {
		data = block.data;
		size = block.size;
	}
	//! Copies pointer and size, not data
	Block& operator=(const Block& block) {
		if (this != &block) {
			data = block.data;
			size = block.size;
		}
		return *this;
	}
	//! Pointer to start of memory location
	uchar* data;
	//! Size in bytes of memory location
	size_t size;
};

MOSH_FCGI_END

#endif
