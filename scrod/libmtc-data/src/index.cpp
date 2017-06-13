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

#include "../lib/index.hpp"
#include "../lib/util.hpp"
#include "../lib/datapacket.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <boost/foreach.hpp>

using namespace mtc::data;
using namespace std;

Index::Index(string file) throw( runtime_error )
{
	file_ = file;
	packetnum = 0;
    buildIndex();
}

Index::Index(const Index& obj) throw( std::runtime_error )
{
    packetnum = obj.packetnum;

    file_ = obj.file_;
    indexFile_ = obj.indexFile_;
    fd_ = ::dup(obj.fd_);
    index_ = obj.index_;
    headers_ = obj.headers_;
}

Index::~Index()
{
    close(fd_);
}

void Index::toFile(int fd) throw( runtime_error )
{
    uint32_t event;
    uint8_t scrod;
    uint8_t channel;
    std::pair<int16_t, off_t> window;
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
                BOOST_FOREACH(window, index_[event][scrod][channel])
                {
                    this->window(event, scrod, channel, window.first).package().toFile(fd);
                }
            }
        }
    }
}

void Index::buildIndex() throw( runtime_error )
{
    index_.clear();
	struct stat fileStat;
	fd_ = ::open(file_.c_str(), O_RDONLY);
	if(fd_ < 0)
        throw( runtime_error(string("Could not open file") + file_) );
	if(fstat(fd_ ,&fileStat) < 0)    
        throw( runtime_error(string("Could not open file") + file_) );
	off_t offset = lseek( fd_, 0, SEEK_CUR );
	int bad = 0;
	try
	{
		while(offset < fileStat.st_size)
		{
			try 
			{
				DataPacket packet(fd_);
				packetnum++;
         	if(packet.type == DataPacket::EVENT_HEADER_WORD)
		      {
			      uint64_t field = packet.scrodId;
		         field = field << 32;
         	   field |= headerEventNumber(packet);
	            headers_.insert(make_pair(field, offset));
	      	}
				if(packet.type != DataPacket::EVENT_WORD)
				{
					offset = lseek( fd_, 0, SEEK_CUR );
					continue;
				}
	            uint32_t eventNumber = windowEventNumber(packet);
	            uint8_t scrodNumber = packet.scrodId;
	
	            if(index_.find(eventNumber) == index_.end())
	                index_[eventNumber] =  ScrodMap();
	            ScrodMap &scrods = index_[eventNumber];
	
	            if(scrods.find(scrodNumber) == scrods.end())
	                scrods[scrodNumber] = ChanMap();
	            ChanMap &scrod = scrods[scrodNumber];

	            vector<Window> windowsInPacket =  windowsFromPacket(packet);

	            for(uint i = 0; i< windowsInPacket.size(); i++)
	            {
	                if(scrod.find(windowsInPacket.at(i).chanId) == scrod.end())
	                    scrod[windowsInPacket.at(i).chanId] = WindowMap();
	                WindowMap& channel = scrod[windowsInPacket.at(i).chanId];
	                channel[windowsInPacket.at(i).windowId] = offset;
	            }
			}
			catch(runtime_error &err) 
			{
            lseek(fd_,offset, SEEK_SET);
            uint32_t buff = 0;
            ::read(fd_, &buff, sizeof(uint32_t));
            buff = 0;
            int read = 4;

					while(read > 3)
	            {
						if(lseek( fd_, 0, SEEK_CUR ) >=  fileStat.st_size - 8)
							break;

						if(buff != DataPacket::PACKET_HEADER)
                  {
							read = ::read( fd_, &buff, sizeof(uint32_t));
							lseek( fd_, -1*(sizeof(uint32_t) -1), SEEK_CUR );
						}
						else
						{
							lseek( fd_, -1*(sizeof(uint32_t)), SEEK_CUR );
							break;
						}
					}
					bad++;
			}
			offset = lseek( fd_, 0, SEEK_CUR );
		}
	}
	catch (runtime_error &e)
	{
		cout << "In file " << file_ << " offset " << offset << " error: " << e.what() << endl;
        throw;
	}
	cout << "Index completed with " << bad << " packets." << endl;
}

vector < uint32_t> Index::events()
{
    vector < uint32_t> ret;
    pair<uint32_t, ScrodMap> item;
    BOOST_FOREACH(item, index_)
    {
        ret.push_back(item.first);
    }
    return ret;
}

vector < uint8_t> Index::scrods(uint32_t event)
{
    vector < uint8_t> ret;
    pair<uint8_t, ChanMap> item;
    if(index_.find(event) == index_.end())
        return ret;
    BOOST_FOREACH(item, index_[event])
    {
        ret.push_back(item.first);
    }
    return ret;
}

vector < uint8_t>  Index::channels(uint32_t event, uint32_t scrod)
{
    vector < uint8_t > ret;
    pair<uint8_t, WindowMap> item;
    if(index_.find(event) == index_.end())
        return ret;
    if(index_[event].find(scrod) == index_[event].end())
        return ret;
    BOOST_FOREACH(item, index_[event][scrod])
    {
        ret.push_back(item.first);
    }
    return ret;
}

vector < uint16_t > Index::windows(uint32_t event, uint32_t scrod, uint8_t chan)
{
    vector < uint16_t > ret;
    pair<uint16_t, off_t> item;
    if(index_.find(event) == index_.end())
        return ret;
    if(index_[event].find(scrod) == index_[event].end())
        return ret;
    if(index_[event][scrod].find(chan) == index_[event][scrod].end())
        return ret;
    BOOST_FOREACH(item, index_[event][scrod][chan])
    {
        ret.push_back(item.first);
    }
    return ret;
}

Window Index::window(uint32_t event, uint32_t scrod, uint8_t chan, uint16_t window)
{
    off_t offset = index_[event][scrod][chan][window];
    lseek( fd_, offset, SEEK_SET );
    DataPacket dp(fd_, false);
    vector<Window> windows = windowsFromPacket(dp);
    Window w;
    BOOST_FOREACH(w, windows)
    {
        if (w.windowId == window)
            return w;
    }
    throw std::runtime_error("Index error");
}

DataPacket Index::getEventHeader(uint32_t event, uint32_t scrod)
{
    uint64_t field = scrod;
    field = field << 32;
    field |= event;
    if(headers_.find(field) == headers_.end())
    {
        throw std::runtime_error("Index error");
    }
    off_t offset = headers_[field];
    lseek( fd_, offset, SEEK_SET );
    return DataPacket(fd_, false);
}

std::vector<DataPacket> Index::getEventHeaders()
{
    std::vector<DataPacket> ret;
    BOOST_FOREACH(EventHeaderMap::value_type &offset, headers_)
    {
        lseek( fd_, offset.second, SEEK_SET );
        ret.push_back(DataPacket(fd_, false));
    }
    return ret;
}

void Index::addPacket(DataPacket &packet)
{
    lseek(fd_, 0, SEEK_END);
    off_t offset = lseek( fd_, 0, SEEK_CUR );
    packet.toFile(fd_);
    packetnum++;
    if(packet.type == DataPacket::EVENT_HEADER_WORD)
    {
        uint64_t field = packet.scrodId;
        field = field << 32;
        field |= headerEventNumber(packet);
        headers_.insert(make_pair(field, offset));
    }
    if(packet.type != DataPacket::EVENT_WORD)
    {
        offset = lseek( fd_, 0, SEEK_CUR );
        return;
    }
    uint32_t eventNumber = windowEventNumber(packet);
    uint8_t scrodNumber = packet.scrodId;

    if(index_.find(eventNumber) == index_.end())
        index_[eventNumber] =  ScrodMap();
    ScrodMap &scrods = index_[eventNumber];

    if(scrods.find(scrodNumber) == scrods.end())
        scrods[scrodNumber] = ChanMap();
    ChanMap &scrod = scrods[scrodNumber];

    vector<Window> windowsInPacket =  windowsFromPacket(packet);

    for(uint i = 0; i< windowsInPacket.size(); i++)
    {
        if(scrod.find(windowsInPacket.at(i).chanId) == scrod.end())
            scrod[windowsInPacket.at(i).chanId] = WindowMap();
        WindowMap& channel = scrod[windowsInPacket.at(i).chanId];
        channel[windowsInPacket.at(i).windowId] = offset;
    }
    offset = lseek( fd_, 0, SEEK_CUR );
}

void Index::addWindow(Window w)
{
    DataPacket d = w.package();
    addPacket(d);
}

