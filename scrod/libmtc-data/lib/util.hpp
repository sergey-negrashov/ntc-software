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

#ifndef MTC_UTIL_DATA_H
#define MTC_UTIL_DATA_H

#include <sys/types.h>
#include <unistd.h> 
#include "datapacket.hpp"
#include <vector>
#include <boost/foreach.hpp>

namespace mtc
{
namespace data
{

///Scrods in use. Don't use this since they change all the time!
static const uint16_t SCROD_IDS[12] = {1,2,3,4,5,6,7,8,9,10,11,12};

///Maximum window in the IRS3B/D used for reference window calculations.
static const int32_t MAX_WINDOW_ID_FOR_IRS2 = 512;


///Pulse descriptor.
typedef struct PulseStruct
{
    ///Scrod ID.
    uint16_t scrod;
    ///Channel ID
    uint8_t chan;
    ///Event number.
    uint event;
    ///Crossing time for cfd.
    float crossingTime;
    ///Crossing window.
    uint window;
    ///Crossing window position.
    float windowPosition;
    ///Maximum value.
    float maxValue;
    ///CFD pulse width.
    float width;
    //Simple charge, 0.5*height*width
    float charge_simp;
	//Integrated charge
	float charge;
	//fixed threshold time
	float fixedThreshTime;

} Pulse;

std::ostream& operator<<(std::ostream& os, const struct PulseStruct& p);
void printPulseHeader(std::ostream& os);

///Representation of the window.
typedef struct WindowStruct
{
    uint16_t windowId;   
	uint8_t chanId;
    uint16_t referenceWindow;
    uint8_t scrodId;
    uint32_t eventNum;
    bool trgBit;
    std::vector<int32_t> values;

    ///Subtract a window values from this one. Nice for pedestal subtraction.
    void subtract(struct WindowStruct &w);
    ///Add a window to this one.
    void add(struct WindowStruct &w);
    ///Divide window values by a scaler.
    void divideScalar(int32_t scalar);
    ///Add a sclaer to all data values.
    void addScalar(int32_t scalar);

    ///Package this window into a DataPacket, so it could be written to disk.
    DataPacket package();
} Window;

///Get all of the windows from packet.
std::vector<Window> windowsFromPacket(DataPacket&);

///Get the reference number.
uint16_t windowReferenceFromPacket(DataPacket&);

///Get event number.
uint32_t windowEventNumber(DataPacket&);

uint32_t headerEventNumber(DataPacket&p);

///Representation of the scrod channel, can be converted to/from SCROD chanel conversion.
typedef struct ChannelStruct
{
    uint8_t row;
    uint8_t col;
    uint8_t chan;

    static ChannelStruct decodeChannel(uint8_t);
    static uint8_t encodeChannel(ChannelStruct);
} Channel;

std::ostream& operator<<(std::ostream& os, const struct ChannelStruct& c);

///Channel on the mTC detector.
typedef struct GlobalChannelStruct
{
    ///Channel ID
    uint8_t channel;
    ///ScrodID.
    uint16_t scrod;
} GlobalChannel;

bool operator<(const GlobalChannelStruct& x, const GlobalChannelStruct& y);
std::ostream& operator<<(std::ostream& os, const struct GlobalChannelStruct &c);

///Removes extension from a file name.
std::string removeExtension( std::string &filename);
}
}

#endif
