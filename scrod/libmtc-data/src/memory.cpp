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

#include "../lib/memory.hpp"
#include "boost/foreach.hpp"
using namespace mtc::data;
using namespace std;

Memory::Memory()
{
    packetnum = 0;
}

Memory::~Memory()
{

}

void Memory::toFile(int fd) throw( std::runtime_error)
{
    uint32_t event;
    uint8_t scrod;
    uint8_t channel;
    Window window;
    std::vector < uint32_t> events = this->events();
    BOOST_FOREACH(event, events)
    {
        std::vector < uint8_t> scrods = this->scrods(event);
        BOOST_FOREACH(scrod, scrods)
        {
            uint64_t field = scrod;
            field = field << 32;
            field |= event;
            if(headers_.find(field) != headers_.end())
            {
                DataPacket eventHeader = this->getEventHeader(event, scrod);
                eventHeader.toFile(fd);
            }
            std::vector <uint8_t> channels = this->channels(event, scrod);
            BOOST_FOREACH(channel, channels)
            {
                BOOST_FOREACH(window, events_[event][scrod][channel])
                {
                    window.package().toFile(fd);
                }
            }
        }
    }
}

std::vector < uint32_t> Memory::events()
{
    vector < uint32_t> ret;
    pair<uint32_t, ScrodMap> item;
    BOOST_FOREACH(item, events_)
    {
        ret.push_back(item.first);
    }
    return ret;
}

std::vector < uint8_t> Memory::scrods(uint32_t event)
{
    vector < uint8_t> ret;
    pair<uint8_t, ChanMap> item;
    if(events_.find(event) == events_.end())
        return ret;
    BOOST_FOREACH(item, events_[event])
    {
        ret.push_back(item.first);
    }
    return ret;
}

std::vector < uint8_t>  Memory::channels(uint32_t event, uint32_t scrod)
{
    vector < uint8_t > ret;
    pair<uint8_t, WindowVector> item;
    if(events_.find(event) == events_.end())
        return ret;
    if(events_[event].find(scrod) == events_[event].end())
        return ret;
    BOOST_FOREACH(item, events_[event][scrod])
    {
        ret.push_back(item.first);
    }
    return ret;

}

std::vector < uint16_t > Memory::windows(uint32_t event, uint32_t scrod, uint8_t chan)
{
    vector < uint16_t > ret;
    Window item;
    if(events_.find(event) == events_.end())
        return ret;
    if(events_[event].find(scrod) == events_[event].end())
        return ret;
    if(events_[event][scrod].find(chan) == events_[event][scrod].end())
        return ret;
    BOOST_FOREACH(item, events_[event][scrod][chan])
    {
        ret.push_back(item.windowId);
    }
    return ret;
}

void Memory::addPacket(DataPacket &p)
{
    if(p.type == p.EVENT_HEADER_WORD)
    {
        uint64_t field = p.scrodId;
        field = field << 32;
        field |= headerEventNumber(p);
        headers_.insert(make_pair(field,DataPacket(p)));
        packetnum++;
        return;
    }
    if(p.type == p.EVENT_WORD)
    {
        vector<Window> windowsInPacket =  windowsFromPacket(p);
        Window w;
        BOOST_FOREACH(w, windowsInPacket)
        {
            this->addWindow(w);
        }
    }
}

void Memory::addWindow(Window w)
{
    if(events_.find(w.eventNum) == events_.end())
    {
        events_.insert(std::make_pair(w.eventNum, ScrodMap()));
    }
    if(events_[w.eventNum].find(w.scrodId) == events_[w.eventNum].end())
    {
        events_[w.eventNum].insert(std::make_pair(w.scrodId, ChanMap()));
    }
    if(events_[w.eventNum][w.scrodId].find(w.chanId) == events_[w.eventNum][w.scrodId].end())
    {
        events_[w.eventNum][w.scrodId].insert(std::make_pair(w.chanId, WindowVector()));
    }
    events_[w.eventNum][w.scrodId][w.chanId].push_back(w);
    packetnum++;
}

Window Memory::window(uint32_t event, uint32_t scrod, uint8_t chan, uint16_t window)
{
    WindowVector v = events_[event][scrod][chan];
    Window w;
    BOOST_FOREACH(w, v)
    {
        if (w.windowId == window)
            return w;
    }
    throw std::runtime_error("Index error");
}

DataPacket Memory::getEventHeader(uint32_t event, uint32_t scrod)
{
    uint64_t field = scrod;
    field = field << 32;
    field |= event;
    if(headers_.find(field) == headers_.end())
    {
        throw std::runtime_error("Index error");
    }
    return headers_[field];
}

std::vector<DataPacket> Memory::getEventHeaders()
{
    std::vector<DataPacket> ret;
    BOOST_FOREACH(EventHeaderMap::value_type &packet, headers_)
    {

        ret.push_back(packet.second);
    }
    return ret;
}
