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

#ifndef DATASTORE_HPP
#define DATASTORE_HPP

#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/foreach.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <boost/foreach.hpp>
#include <algorithm>

#include "util.hpp"
#include "pmtmap.hpp"
namespace mtc
{
namespace data
{

/**
 * @brief The DataStore Abstract class represents a collection of windows indexable by event, scrod, channel and ID.
 */
class DataStore
{
public:
    ///Store in file discriptor.
    virtual void toFile(int fd) = 0;
    ///List of events.
    virtual std::vector < uint32_t> events() = 0;
    ///List of scrods.
    virtual std::vector < uint8_t> scrods(uint32_t event) = 0;
    ///List of channels.
    virtual std::vector < uint8_t>  channels(uint32_t event, uint32_t scrod) = 0;
    ///List of windows.
    virtual std::vector < uint16_t > windows(uint32_t event, uint32_t scrod, uint8_t chan) = 0;

    ///Returns a window object indexed by event, scrod, channel and ID.
    virtual Window window(uint32_t event, uint32_t scrod, uint8_t chan, uint16_t window) = 0;

    ///Get an event header packet for the event and scrod.
    virtual DataPacket getEventHeader(uint32_t event, uint32_t scrod) = 0;

    ///Get all event headers.
    virtual std::vector<DataPacket> getEventHeaders() = 0;

    ///Add packet to store.
    virtual void addPacket(DataPacket &d) = 0;
    ///Packetize a window and add it to the store..
    virtual void addWindow(Window w) = 0;

    ///Number of packets.
    int packetnum;

    ///Export to a Shige text format.
    void exportToText(std::string fname)
    {
        using namespace std;
        PmtMap pmts;
        try
        {
            pmts.parse();
        }
        catch(runtime_error &e)
        {
            cout << "Pmt to scrod file not available " << e.what() << endl;
        }
        uint32_t event;
        uint8_t scrod;
        uint8_t channel;
        uint16_t window;
        ofstream outputFile;
        outputFile.open(fname.c_str(), ios::out|ios::trunc);
        if(!outputFile.is_open())
        {
            cerr << "Could not open file " << fname << endl;
            throw runtime_error("Could not open file" + fname);
        }
        vector < uint32_t> events = this->events();

        time_t now = time(0);
        struct tm  tmstruct;
        char buf[80];
        tmstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tmstruct);

        outputFile << "##################################" << endl;
        outputFile << "# Created: " << buf << endl;
        outputFile << "# Events: " << events.size() << endl;
        outputFile << "##################################" << endl;
        outputFile << endl;

        BOOST_FOREACH(event, events)
        {
            outputFile << "# Event: " << event << endl;
            std::vector < uint8_t> scrods = this->scrods(event);
            BOOST_FOREACH(scrod, scrods)
            {

                std::vector <uint8_t> channels = this->channels(event, scrod);
                BOOST_FOREACH(channel, channels)
                {
                    std::vector < uint16_t> windows = this->windows(event, scrod, channel);
                    Window w = this->window(event, scrod, channel, windows[0]);
                    uint16_t refWindow = w.referenceWindow;
                    GlobalChannel gc;
                    gc.channel = channel;
                    gc.scrod = scrod;

                    outputFile << "# " << gc << endl;

                    outputFile << "# " << pmts.getPmtChannel(gc) << endl ;

                    outputFile << "# Reference Window:" << (int)refWindow << endl;
                    try
                    {
                        DataPacket eventHeaderPacket = getEventHeader(event, scrod);
                        uint32_t localTimestamp = eventHeaderPacket.data[6];
                        outputFile << "# Local Trigger Time: " << localTimestamp << endl;
                    }
                    catch(std::runtime_error & e)
                    {
                        outputFile << "# Local Trigger Time: -1" << endl;
                    }
                    std::sort(windows.begin(), windows.end());
                    BOOST_FOREACH(window, windows)
                    {
                        if(window < refWindow)
                            continue;
                        w = this->window(event, scrod, channel, window);
                        outputFile << "# W: " << (int) w.windowId << endl;
                        int32_t value;
                        BOOST_FOREACH(value, w.values)
                        {
                            outputFile << value << " ";
                        }
                        outputFile << endl << endl;
                    }

                    BOOST_FOREACH(window, windows)
                    {
                        if(window >= refWindow )
                            continue;
                        w = this->window(event, scrod, channel, window);
                        outputFile << "# W: " << (int) w.windowId << endl;
                        int32_t value;
                        BOOST_FOREACH(value, w.values)
                        {
                            outputFile << value << " ";
                        }
                        outputFile << endl << endl;
                    }

                }
            }
        }
        outputFile.close();
    }
};
}
}
#endif // DATASTORE_HPP
