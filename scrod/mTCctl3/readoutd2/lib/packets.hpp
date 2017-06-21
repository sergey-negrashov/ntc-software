/*
 * This file is part of readoutd.
 *
 * readoutd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * readoutd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with readoutd.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright Sergey Negrashov 2014.
*/

#ifndef PACKETINTERFACE_HPP
#define PACKETINTERFACE_HPP

#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <boost/thread.hpp>
#include <sys/ioctl.h>
#include "readoutProtocol.hpp"
#include "common.hpp"
#include <iostream>
using namespace std;

#pragma pack(push,1)
static const uint32_t PACKET_HEADER =  0x00BE11E2;

static const uint32_t CMD_HEADER =  0x646f6974;

static const uint32_t CMD_PING_WORD = 0x70696e67;

static const uint32_t CMD_READ_REGISTER_WORD = 0x72656164;

static const uint32_t CMD_WRITE_REGISTER_WORD = 0x72697465;

static const uint32_t CMD_RESPONSE_SUCCESS_WORD = 0x6F6B6179;

static const uint32_t CMD_RESPONSE_FAILED_WORD = 0x7768613f;

static const uint32_t EVENT_HEADER_WORD = 0x65766e74;

static const uint32_t EVENT_WORD = 0x77617665;

struct EventHeader
{
    uint32_t belleHeader;
   uint32_t size;   //-2
    uint32_t packetType;
    uint32_t id;
    uint32_t proto;
    uint32_t event;
    uint32_t triggerType;
    uint32_t eventFlags;
    uint32_t numPackets;
    uint32_t auxPackets;
    uint32_t checksum; //0 to N-1
};

/**
 * @brief The RegisterCommand struct as defined in the scrod firmware.
 */
struct RegisterCommand
{
    uint32_t belleHeader;
    uint32_t size;   //-2
    uint32_t packetType;
    uint32_t dest;
    uint32_t id;

    uint32_t registerCommand;
    uint32_t valueAddress;  //Highbits are value, low bits are address;
    uint32_t commandChecksum;

    uint32_t checksum; //0 to N-1
};

/**
 * @brief The PingCommand struct as defined in the scrod firmware.
 */
struct PingCommand
{
    uint32_t belleHeader;
    uint32_t size;   //-2
    uint32_t packetType;
    uint32_t dest;

    uint32_t id;
    uint32_t pingCommand;
    uint32_t commandChecksum;

    uint32_t checksum; //0 to N-1
};

/**
 * @brief The PongResponce struct as defined in the scrod firmware.
 */
struct PongResponce
{
    uint32_t belleHeader;
    uint32_t size;   //-2
    uint32_t packetType;
    uint32_t dest;
    uint32_t id;
    uint32_t pingCommand;
    uint32_t checksum; //0 to N-1
};

/**
 * @brief The RegisterResponce struct  as defined in the scrod firmware.
 */
struct RegisterResponce
{
    uint32_t belleHeader;
    uint32_t size;   //-2
    uint32_t packetType;
    uint32_t dest;
    uint32_t id;
    uint32_t command;
    uint32_t valueAddress;
    uint32_t checksum; //0 to N-1
};

/**
 * @brief The FailedResponce struct as defined in the scrod firmware.
 */
struct FailedResponce
{
    uint32_t belleHeader;
    uint32_t size;   //-2
    uint32_t packetType;
    uint32_t dest;
    uint32_t id;
    uint32_t command;
    uint32_t valueAddress;
    uint32_t checksum; //0 to N-2
};
#pragma pack(pop)
/**
 * @brief checksum checksums the packet.
 * @param packet packet to checksum.
 * @param size packet size in words.
 * @return checksum.
 */
inline uint32_t checksum(uint32_t *packet, int size)
{
    uint32_t sum = 0;
    for(int i = 0; i< size; i++)
    {
        sum += packet[i];
    }
    return sum;
}

/**
 * @brief checkChecksum checks the checksum of the packet. Assumes that the checksum is located at size -1.
 * @param packet packet to check.
 * @param size size of packet.
 * @return true if checksum is ok, false otherwise.
 */
inline bool checkChecksum(uint32_t *packet, int size)
{
    return checksum(packet, size - 1) == packet[size - 1];
}

/**
 * @brief readPacket reads a packet over fiber.
 */
inline std::vector<uint32_t>  readPacket(int fd, int buffSize, bool check_checksum = true)
{
    int size = 0;
    uint32_t reportedSize;
    std::vector<uint32_t> buffer(buffSize/4);
    size = read(fd, buffer.data(), buffSize);
    if(size < 0)
        throw std::runtime_error("Packet: timed out");
    if(size < 32)
        throw std::runtime_error("Packet: packet too small");
    if(buffer[0] != PACKET_HEADER)
        throw std::runtime_error("Packet: bad header");
    reportedSize = buffer[1];
    if(reportedSize +2 != size/sizeof(uint32_t))
        throw std::runtime_error("packet wrong size");
    if( check_checksum && (!checkChecksum(buffer.data(), reportedSize +2)))
        throw std::runtime_error("Packet: bad checksum" );
    return buffer;
}

/**
 * @brief readPacket reads a packet over fiber.
 */
inline uint32_t* readPacketHeap(int fd, int buffSize, bool check_checksum = true)
{
    int size = 0;
    uint32_t reportedSize;
    uint32_t *buffer = new uint32_t[buffSize/4];
    size = read(fd, buffer, buffSize);
    if(size < 0)
    {
        for (int blah = 0; blah < size/4; ++blah) {
            std::cerr << hex << buffer[blah] << dec << std::endl;
        }
        delete buffer;
        throw std::runtime_error("Packet: timed out");
    }
    if(size < 32)
    {
        for (int blah = 0; blah < size/4; ++blah) {
            std::cerr << hex << buffer[blah] << dec << std::endl;
        }
        delete buffer;
        throw std::runtime_error("Packet: packet too small");
    }
    if(buffer[0] != PACKET_HEADER)
    {
        for (int blah = 0; blah < size/4; ++blah) {
            std::cerr << hex << buffer[blah] << dec << std::endl;
        }
        delete buffer;
        throw std::runtime_error("Packet: bad header");
    }
    reportedSize = buffer[1];
    if(reportedSize +2 != size/sizeof(uint32_t))
    {
        for (int blah = 0; blah < size/4; ++blah) {
            std::cerr << hex << buffer[blah] << dec << std::endl;
        }
        delete buffer;
        throw std::runtime_error("packet wrong size");
    }
    if( check_checksum && (!checkChecksum(buffer, reportedSize +2)))
    {
        for (int blah = 0; blah < size/4; ++blah) {
            std::cerr << hex << buffer[blah] << dec << std::endl;
        }
        delete buffer;
        throw std::runtime_error("Packet: bad checksum" );
    }
    return buffer;
}

/**
 * @brief setRegister sends the set register command.
 * @param fd file descriptor to the altix driver, locked and ready to go.
 * @param reg register address to set.
 * @param data value to set the register to.
 * @param sequence sequence number for the command.
 * @return true if ok, false otherwise.
 */
inline DspResponce setRegister(int fd, int reg, int data, int sequence)
{
    RegisterCommand c;
    c.belleHeader = PACKET_HEADER;
    c.size =  sizeof(RegisterCommand)/4 - 2;
    c.packetType = CMD_HEADER;
    c.dest = 0x0;
    c.id= sequence;

    c.registerCommand = CMD_WRITE_REGISTER_WORD;
    c.valueAddress = 0xFFFF & reg;
    c.valueAddress |= (data & 0xFFFF) << 16;
    c.commandChecksum = c.valueAddress + c.registerCommand + sequence;
    c.checksum = checksum(((uint32_t*)&c), sizeof(c)/4  - 1);
    if(::write(fd, &c, sizeof(c)) < 0)
        throw std::runtime_error("Packet: Could not write packet" );
    DspResponce ret;
    ret.address = reg;
    ret.ok = false;
    ret.arg = TIMEOUT_ERROR;

    std::vector<uint32_t> packet = readPacket(fd, sizeof(RegisterResponce),CHECK_PACKET_CHECKSUM);

    if(packet.size()*4 < sizeof(RegisterResponce))
        return ret;
    RegisterResponce *rsp =  (RegisterResponce *)packet.data();
    ret.address = rsp->valueAddress & 0xFFFF;;
    ret.arg = (rsp->valueAddress >> 16) & 0xFFFF;
    ret.card = 0;
    ret.chan = 0;
    ret.ok = (rsp->packetType == CMD_RESPONSE_SUCCESS_WORD);
    return ret;

}

/**
 * @brief getRegister get a register value command.
 * @param fd file descriptor to the altix driver, locked and ready to go.
 * @param reg register address.
 * @param sequence sequence number for this command
 * @return true if ok, false otherwise.
 */
inline DspResponce getRegister(int fd, int reg, int sequence)
{
    RegisterCommand c;
    c.belleHeader = PACKET_HEADER;
    c.size =  sizeof(RegisterCommand)/4 - 2;
    c.packetType = CMD_HEADER;
    c.dest = 0x0;
    c.id= sequence;

    c.registerCommand = CMD_READ_REGISTER_WORD;
    c.valueAddress = 0xFFFF & reg;
    c.commandChecksum = c.valueAddress + c.registerCommand + sequence;
    c.checksum = checksum(((uint32_t*)&c), sizeof(c)/4  - 1);
    int count;
    if((count = ::write(fd, &c, sizeof(c))) < 0)
        throw std::runtime_error("Packet: Could not write packet" );

    DspResponce ret;
    ret.address = reg;
    ret.ok = false;
    ret.arg = TIMEOUT_ERROR;
    std::vector<uint32_t> packet = readPacket(fd, sizeof(RegisterResponce),CHECK_PACKET_CHECKSUM);
    if(packet.size()*4 < sizeof(RegisterResponce))
        return ret;
    RegisterResponce *rsp =  (RegisterResponce *)packet.data();
    ret.address = rsp->valueAddress & 0xFFFF;;
    ret.arg = (rsp->valueAddress >> 16) & 0xFFFF;
    ret.ok = (rsp->packetType == CMD_RESPONSE_SUCCESS_WORD);
    return ret;
}

#endif // PACKETS_HPP
