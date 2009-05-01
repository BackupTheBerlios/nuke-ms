// msglayer.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "bytes.hpp"
#include "msglayer.hpp"

using namespace nuke_ms;


UnknownMessageLayer::UnknownMessageLayer(
    DataOwnership _memblock,
    BasicMessageLayer::const_data_iterator _data_it,
    std::size_t _data_size,
    bool new_memory_block
)
    : memblock(_memblock), data_it(_data_it), data_size(_data_size)
{

}


// overriding base class version
std::size_t UnknownMessageLayer::getSerializedSize() const throw()
{
    // return the size of the memory block
    return data_size;
}

// overriding base class version
void UnknownMessageLayer::fillSerialized(data_iterator buffer) const throw()
{
    // copy the maintained data into the specified buffer
    std::copy(
        data_it,
        data_it + data_size,
        buffer
    );
}

