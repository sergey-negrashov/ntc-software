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

#include "../lib/pmtmap.hpp"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace mtc::data;

std::ostream& mtc::data::operator<<(std::ostream& os, const struct PmtChannelStruct& c)
{
    if(c.pmt == -1)
        os << "Pmt: Unknown";
    else
        os << "Pmt: " << (int)c.pmt << "; Row: " <<
              (int)c.row << "; Col: " << (int)c.col;
    return os;
}

PmtMap::PmtMap()
{
    ok = false;
}

PmtChannel PmtMap::getPmtChannel(GlobalChannel gc)
{
    PmtChannel out;
    uint32_t gcInt = (0xFFFF & gc.scrod << 8) | (gc.channel & 0xFF);

    if(pmts.find(gcInt) == pmts.end())
        out.pmt = -1;
    else
        out = pmts[gcInt];
    return out;
}

void PmtMap::parse() throw(std::runtime_error)
{
    ifstream configFile(FRONT_BOARD_AND_PMT_LIST_PATH.c_str(), ios::in);
    if(!configFile.is_open())
    {
        throw runtime_error("Could not open pmt map master list at " + FRONT_BOARD_AND_PMT_LIST_PATH);
    }

    int lineCounter = 0;
    Channel chan;
    chan.row = 0;
    chan.col = 0;
    chan.chan = 0;
    for( std::string line; getline( configFile, line ); )
    {
        lineCounter++;
        boost::algorithm::trim(line);
        if(line.length() == 0)
            continue;
        if(line.at(0) == '#')
            continue;
        std::istringstream iss(line);
        int16_t mapItems[8];
        for(int i = 0; i< 8; i++)
        {
            if (!(iss >> mapItems[i]))
            {
                throw std::runtime_error("Format error on line " + boost::lexical_cast<std::string>(lineCounter) + " file " + FRONT_BOARD_AND_PMT_LIST_PATH);
            }
        }
        GlobalChannel gc;
        if (!(iss >> gc.scrod))
        {
            throw std::runtime_error("Format error on line " + boost::lexical_cast<std::string>(lineCounter) + " file " + FRONT_BOARD_AND_PMT_LIST_PATH);
        }
        for(int i = 0; i< 8; i++)
        {

            chan.chan = i;
            PmtChannel pmt;
            pmt.pmt = mapItems[i]/100;
            pmt.row = (mapItems[i] %100)/10;
            pmt.col = (mapItems[i] %10);
            gc.channel = Channel::encodeChannel(chan);
            uint32_t gcInt = (0xFFFF & gc.scrod << 8) | (gc.channel & 0xFF);

            pmts[gcInt] =  pmt;
        }
        chan.chan = 0;
        chan.col++;
        if(chan.col == 4)
        {
            chan.col = 0;
            chan.row++;
        }
        if(chan.row == 4)
        {
            chan.row = 0;
        }
    }
    ok = true;
}
