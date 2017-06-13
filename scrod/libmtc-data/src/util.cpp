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

#include "../lib/util.hpp"
#include <ostream>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

namespace mtc
{
namespace data
{
std::vector<Window> windowsFromPacket(DataPacket& p)
{
	std::vector<Window>	ret;
	if(p.type != p.EVENT_WORD)
		return ret;
    uint16_t reference = p.data[0];
    uint32_t eventNum = p.data[1];
    uint segments = p.data[2];
    uint pos = 3;
    for(size_t i = 0; i< segments; i++)
    {
        Window next;
        next.chanId = (p.data[pos + 0] >> 9) & 0x7F;
        next.windowId = p.data[pos + 0] & 0x1ff;
        next.trgBit = (p.data[pos+0] >> 16) & 0x1;
        next.referenceWindow = reference;
        next.eventNum = eventNum;
        next.scrodId = p.scrodId;
        uint size = p.data[pos + 1];

        for(size_t j = 0; j < size/2; j++)
        {
            next.values.push_back((int16_t)(p.data[pos + j +2] & 0xFFFF));
            next.values.push_back((int16_t)(p.data[pos + j +2] >> 16));
        }

        ret.push_back(next);
        pos += size/2 + 2;
    }
	return ret;
}

void WindowStruct::subtract(WindowStruct &w)
{
    for(uint i = 0; i< values.size(); i++)
    {
        values[i] -= w.values[i];
    }
}

void WindowStruct::add(struct WindowStruct &w)
{
    for(uint i = 0; i< values.size(); i++)
    {
        values[i] += w.values[i];
    }
}

void WindowStruct::divideScalar(int32_t scalar)
{
    for(uint i = 0; i< values.size(); i++)
    {
        values[i] /= scalar;
    }
}

void WindowStruct::addScalar(int32_t scalar)
{
    for(uint i = 0; i< values.size(); i++)
    {
        values[i] += scalar;
    }
}

DataPacket WindowStruct::package()
{
    DataPacket p;
    p.isNull = false;
    p.type = p.EVENT_WORD;
    p.scrodId = this->scrodId;
    std::vector<uint32_t> data;
    data.push_back(this->referenceWindow);  //reference window field;
    data.push_back(this->eventNum);         //event number
    data.push_back(1);                  //segments
    uint32_t windowAddress = this->chanId;
    windowAddress = (windowAddress << 9) | (this->windowId & 0x1ff) | ((this->trgBit & 0x1) << 16);
    data.push_back(windowAddress);      //address
    data.push_back(64);                 //should this be 512? noted July 2016
    for(size_t i = 0; i < this->values.size()/2; i++)
    {
        uint32_t next = ((int16_t)this->values[i*2]) & 0xFFFF;
        next |= (0xFFFF & ((int16_t)this->values[i*2 + 1])) << 16;
        data.push_back(next);
    }
    p.data = new uint32_t[data.size()];

    for(size_t i = 0; i < data.size(); i++)
    {
        p.data[i] = data[i];
    }
    p.size = data.size();
    return p;
}

uint16_t windowReferenceFromPacket(DataPacket& p)
{
	if(p.type != p.EVENT_WORD)
		return 0;
	return p.data[0];
}

uint32_t windowEventNumber(DataPacket&p)
{
	if(p.type != p.EVENT_WORD)
		return 0;
	return p.data[1];	
}

uint32_t headerEventNumber(DataPacket&p)
{
    if(p.type != p.EVENT_HEADER_WORD)
        return 0;
    return p.data[1];
}

Channel Channel::decodeChannel(uint8_t c)
{
    Channel ret;
    ret.chan = c & 0x7;
    ret.row = (c >> 3) & 0x3;
    ret.col = (c >> 5) & 0x3;
    return ret;
}

uint8_t Channel::encodeChannel(Channel c)
{
    return ((c.chan & 0x7)  | ((0x3 & c.row) << 3) | (( 0x3 & c.col) << 5));
}


std::ostream& operator<<(std::ostream& os, const struct ChannelStruct& c)
{
    os << "R: " << (int)c.row << "; C: " <<
          (int)c.col << "; CH: " << (int)c.chan;
    return os;
}

std::ostream& operator<<(std::ostream& os, const struct PulseStruct& p)
{
    os << (uint)p.event << "\t" << (uint)p.scrod << "\t" << Channel::decodeChannel(p.chan) << "\t" << (float)p.crossingTime << "\t\t"
          << (uint)p.maxValue << "\t" << (uint)p.width << "\t" << (uint)p.window << "." << (uint)p.windowPosition;
    return os;
}

void printPulseHeader(std::ostream& os)
{
    os << "#Event" << "\t" << "Scrod" << "\t" << "Channel" << "\t\t\t" << "Crossing time(ns)" << "\t"
          << "Max Value" << "\t" << "Width" << "\t" << "Window Position" << std::endl;
}

bool operator<(const GlobalChannelStruct& x, const GlobalChannelStruct& y)
{
    /*
    if(x.channel != y.channel)
        return x.channel < y.channel;
    return x.scrod < y.scrod;
    */
    return boost::make_tuple(x.channel,x.scrod) < boost::make_tuple(y.channel,y.scrod);
}

std::ostream& operator<<(std::ostream& os, const struct GlobalChannelStruct &gc)
{
    Channel c = Channel::decodeChannel(gc.channel);
    os << "Scrod: " << (int)gc.scrod << "; " << c;
    return os;
}

std::string removeExtension(std::string &fullname)
{
    size_t lastindex = fullname.find_last_of("."); 
    std::string rawname = fullname.substr(0, lastindex);
    return rawname;
    
    //size_t lastdot = filename.find_last_of(".");
    //if (lastdot == std::string::npos) return filename;
    //return filename.substr(0, lastdot);
}


}
}
