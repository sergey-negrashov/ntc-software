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

#include "../lib/channelignoremask.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace mtc::data;

ChannelIgnoreMask::ChannelIgnoreMask()
{
}

void ChannelIgnoreMask::parse() throw(std::runtime_error)
{
    ifstream configFile(IGNORE_LIST_PATH.c_str());
    if(!configFile.is_open())
    {
        throw runtime_error("Could not open channel ignore mask");
    }
    int lineCounter = 0;
    for( std::string line; getline( configFile, line ); )
    {
        lineCounter++;
        boost::algorithm::trim(line);
        if(line.length() == 0)
            continue;
        if(line.at(0) == '#')
            continue;
        std::istringstream iss(line);
        int scrod;
        Channel chan;
        int window;
        if (!(iss >> scrod >> chan.row >> chan.col >> chan.chan))
        {
            throw std::runtime_error("Format error on line " + boost::lexical_cast<std::string>(lineCounter) + " file " + IGNORE_LIST_PATH);
        }
        if(!(iss>>window))
        {
            for(int i = 0; i< MAX_WINDOW_ID_FOR_IRS2; i++)
            {
                maskList.push_back((std::make_pair(scrod,((uint)Channel::encodeChannel(chan) << 8 | i))));
               // cout << scrod << " "<<( (uint)Channel::encodeChannel(chan) << 8 | i) << endl;
            }
        }
        else
        {
            maskList.push_back(std::make_pair(scrod,((uint)Channel::encodeChannel(chan) << 8 | (uint)window)));
            //cout << scrod << " " << " " << ((uint)Channel::encodeChannel(chan) << 8 | (uint)window) << endl;
        }
    }
}
