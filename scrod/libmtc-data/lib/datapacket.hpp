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

#ifndef MTC_DATA_DATAPACKET_H
#define MTC_DATA_DATAPACKET_H
#include <vector>
#include <stdint.h>
#include <stdexcept>

namespace mtc
{
namespace data
{
/**
 * @brief The DataPacket class represents a scrod packet.
 */
class DataPacket
{
public:
    ///Dont use this constructor.
    DataPacket();
    ///Copy constructor.
    DataPacket( const DataPacket& other );
    ///Reads packet from file description.
    DataPacket(int fd, bool checkChecksum = true) throw( std::runtime_error );
    ///Parses a packer from a buffer. Buffer memory ownership is not changed, as the data is copied.
    DataPacket(uint32_t* buffer, bool checkChecksum = true) throw( std::runtime_error );
    ///Parse packet from a vector of 32bit words.
    DataPacket(std::vector<uint32_t> buffer, bool checkChecksum = true) throw( std::runtime_error );

    ~DataPacket();
    ///Write pactet to file descriptor.
    int toFile(int fd);
    //data
    bool isNull;
    /// Pointer to the packet data minus the header(4 32bit words below)
    uint32_t* data;
    ///Type field.
    uint32_t type;
    ///Size field
    uint32_t size;
    ///Scrod ID field.
    uint8_t scrodId;
    /// Full ID including revision.
    uint32_t fullID;
    //Packet types.
    static const uint32_t PACKET_HEADER =  0x00BE11E2;
    static const uint32_t CMD_HEADER =  0x646f6974;
    static const uint32_t CMD_PING_WORD = 0x70696e67;
    static const uint32_t CMD_READ_REGISTER_WORD = 0x72656164;
    static const uint32_t CMD_WRITE_REGISTER_WORD = 0x72697465;
    static const uint32_t CMD_RESPONSE_SUCCESS_WORD = 0x6F6B6179;
    static const uint32_t CMD_RESPONSE_FAILED_WORD = 0x7768613f;
    static const uint32_t EVENT_HEADER_WORD = 0x65766e74;
    static const uint32_t EVENT_WORD = 0x77617665;

private:
    void init_(uint32_t*,bool checkChecksum_) throw( std::runtime_error );
};
}
}
#endif // DATAPACKET_H
