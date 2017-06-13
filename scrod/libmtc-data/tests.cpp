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

#include "lib/dataoperations.hpp"
#include "lib/channelignoremask.hpp"
#include "lib/pmtmap.hpp"
#include "lib/scrodmap.hpp"
#include "lib/pmtfacemap.hpp"
#include <iostream>

using namespace std;
using namespace mtc::data;

int main(int argc, char** argv)
{
    cout  << "Testing the channel mask" << endl << flush;
    ChannelIgnoreMask ignoreMask;
    ignoreMask.parse();

    cout << "Testing scrod to pmt map" << endl << flush;
    PmtMap map;
    map.parse();
    GlobalChannel c;
    Channel chan;
    chan.row = 1;
    chan.col = 2;
    chan.chan = 0;
    c.scrod = 102;
    c.channel = chan.encodeChannel(chan);
    cout << map.getPmtChannel(c) << endl;
    cout << "Testing scrod to side map" << endl << flush;
    ScrodPositionMap smap;
    smap.parse();
    cout << smap.getPosition(2) << endl;

    PmtPositionMap pmap;
    pmap.parse();
    cout << pmap.getPosition(13) << endl;

    if(argc >= 3)
    {
        cout  << "Testing data indexing" << endl << flush;
        Index data(argv[1]);

        Index ped(argv[2]);

        cout  << "Testing data to text conversion" << endl << flush;
        data.exportToText("data.txt");
        ped.exportToText("ped.txt");

        cout  << "Testing pedistal subtraction" << endl << flush;
        Memory m = subtractPedistal(data, ped);
        cout  << "Testing pedistal subtraction" << endl << flush;
        int fd = ::open("subtracted.dat", O_WRONLY | O_TRUNC | O_CREAT, 0666);
        cout  << "Testing extracting to binary test" << endl << flush;
        m.toFile(fd);
        cout  << "Testing memory to text conversion" << endl << flush;
        m.exportToText("subtracted.txt");

        cout  << "Testing prunning of dead channels" << endl << flush;
        Memory mp = pruneChannels(m, ignoreMask);
        mp.exportToText("subtracted_pruned.txt");

        cout  << "Testing avereging for the pedistals" << endl << flush;
        std::vector<DataStore*> vec;
        vec.push_back(&data);
        vec.push_back(&data);
        Memory mpa = averageSamples(vec);
        mpa.exportToText("data_averaged.txt");

        cout << "Testing curve generation" << endl << flush;
        int32_t event = mpa.events()[0];
        uint8_t scrod = mpa.scrods(event)[0];
        uint8_t channel =  mpa.channels(event,scrod)[0];
        Curve curve =generateCurveNoTiming(mpa,event,scrod,channel, map);
        curve.exportToGnuplot("testcurve.txt");
    }
    return 0;
}
