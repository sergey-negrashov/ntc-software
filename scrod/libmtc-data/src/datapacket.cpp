/*
 *  This file is part of libmtc-data.
 *
 *  libmtc-data is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libmtc-data is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libmtc-data.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright Sergey Negrashov 2014.
*/

#include "../lib/datapacket.hpp"
#include <unistd.h>
#include <string.h>
using namespace mtc::data;

inline uint32_t checksum(uint32_t *packet, int size)
{
    uint32_t sum = 0;
    for(int i = 0; i< size; i++)
    {
        sum += packet[i];
    }
    return sum;
}

inline bool checkChecksum(uint32_t *packet, int size)
{
    return checksum(packet, size - 1) == packet[size - 1];
}

DataPacket::DataPacket()
{
    isNull = true;
}

DataPacket::DataPacket( const DataPacket& other )
{
    isNull = other.isNull;
    if(isNull)
        return;
    scrodId = other.scrodId;
    size = other.size;
    type = other.type;
    fullID = other.fullID;
    data = new uint32_t[other.size];
    memmove(data, other.data, size*sizeof(uint32_t));
}

DataPacket::DataPacket(int fd, bool check) throw( std::runtime_error )
{
    uint32_t header[2] = {0};
    int size = ::read(fd, header, sizeof(uint32_t)*2);
    if(size != sizeof(header))
        throw std::runtime_error("Bad File: not enogth data");
    if(header[0] != PACKET_HEADER)
    {
        lseek64(fd, -2, SEEK_CUR);
        throw std::runtime_error("Bad Header: not 0x00BE11E2");
    }
    if(header[1] > (1 << 27))
    {
        lseek64(fd, -2, SEEK_CUR);
        throw std::runtime_error("Bad Header: packet too big");
    }
    size = header[1];
    uint32_t buffer[size+2];
    buffer[0] = header[0];
    buffer[1] = header[1];
    ::read(fd, (buffer + 2), size*sizeof(uint32_t));
    init_(buffer, check);
}

DataPacket::DataPacket(uint32_t* buffer, bool check) throw( std::runtime_error )
{
    if(buffer[0] != PACKET_HEADER)
        throw std::runtime_error("Bad Header: not 0x00BE11E2");
    if(buffer[1] > (1 << 23))
    {
        throw std::runtime_error("Bad Header: packet too big");
    }
    init_(buffer, check);
}

DataPacket::DataPacket(std::vector<uint32_t> buffer, bool check) throw( std::runtime_error )
{
    if(buffer[0] != PACKET_HEADER)
        throw std::runtime_error("Bad Header: not 0x00BE11E2");
    if(buffer[1] > (1 << 27))
        throw std::runtime_error("Bad Header: packet too big");
    init_(buffer.data(), check);
}

DataPacket::~DataPacket()
{
    if(!isNull)
        delete [] data;
}

void DataPacket::init_(uint32_t* buffer, bool check) throw( std::runtime_error )
{
    isNull = false;
    type = buffer[2];
    scrodId = buffer[3] >> 16;
    fullID = buffer[3];
    size = buffer[1] - 3;   //exclude checksum type and scrodId.
    
    if(check)
        if(!checkChecksum(buffer, buffer[1] + 2))
            throw std::runtime_error("Bad checksum");
	
    data = new uint32_t[size];
    ::memmove(data, buffer + 4, size*sizeof(uint32_t));
}

int DataPacket::toFile(int fd)
{
    /*
    uint32_t sum = PACKET_HEADER;
    std::vector<uint32_t> buff(size + 5);
    buff[0] = PACKET_HEADER;
    buff[1] = size + 3;
    sum += data[1];
    buff[2] = type;
    sum += data[2];
    buff[3] = fullID;
    sum += data[3];
    for(size_t i = 0; i < size -1;i++)
    {
        buff[4+i] = data[i];
        sum += data[i];
    }
    buff[size+4] = sum;

    return ::write(fd, buff.data(), 4*buff.size());
    */

    int total = 0;
    uint32_t header = PACKET_HEADER;
    uint32_t actualSize = size +3;
    uint32_t cksum = checksum(data, size);
    uint32_t actualId = scrodId << 16;
    cksum += type;
    cksum += actualSize;
    cksum += header;
    cksum += actualId;
    total += ::write(fd, &header, sizeof(uint32_t));
    total += ::write(fd, &actualSize, sizeof(uint32_t));
    total += ::write(fd, &type, sizeof(uint32_t));
    total += ::write(fd, &actualId, sizeof(uint32_t));
    total += ::write(fd, data, size*sizeof(uint32_t));
    total += ::write(fd, &cksum, sizeof(uint32_t));
    return total;
}
