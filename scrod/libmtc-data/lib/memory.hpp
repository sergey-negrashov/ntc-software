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

#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <vector>
#include <stdint.h>
#include <string>
#include <stdexcept>
#include <map>

#include <sys/types.h>
#include <unistd.h>

#include "util.hpp"
#include "datapacket.hpp"
#include "datastore.hpp"

namespace mtc
{
namespace data
{
/**
 * @brief The Memory class Represents a collection of windows in memory.
 */
class Memory: public DataStore
{
public:

    Memory();
    ~Memory();

    void toFile(int fd) throw( std::runtime_error );

    std::vector < uint32_t> events();
    std::vector < uint8_t> scrods(uint32_t event);
    std::vector < uint8_t>  channels(uint32_t event, uint32_t scrod);
    std::vector < uint16_t > windows(uint32_t event, uint32_t scrod, uint8_t chan);

    void addPacket(DataPacket &d);
    void addWindow(Window w);

    Window window(uint32_t event, uint32_t scrod, uint8_t chan, uint16_t window);
    DataPacket getEventHeader(uint32_t event, uint32_t scrod);
    std::vector<DataPacket> getEventHeaders();
    int packetnum;

private:
    typedef std::vector<Window> WindowVector;
    typedef std::map<uint8_t, WindowVector> ChanMap;
    typedef std::map<uint8_t, ChanMap> ScrodMap;
    typedef std::map<uint32_t, ScrodMap> EventMap;
    typedef std::map<uint64_t, DataPacket> EventHeaderMap;

    EventMap events_;
    EventHeaderMap headers_;
};

}
}
#endif // MEMORY_HPP
