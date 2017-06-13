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
#include "../lib/curve.hpp"
#include "../lib/util.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <exception>
#include <boost/foreach.hpp>
#include <sstream>

using namespace std;

namespace mtc
{
namespace data
{

void Curve::exportToGnuplot(std::string fname)
{
    ofstream outputFile;
    outputFile.open(fname.c_str(), ios::out|ios::trunc);
    if(!outputFile.is_open())
    {
        cerr << "Could not open file " << fname << endl;
        throw runtime_error("Could not open file" + fname);
    }
    Channel chan = Channel::decodeChannel(this->channel);
    outputFile << "# Event: " << this->event << endl;
    outputFile << "# Scrod: " << (int)this->scrod << "; " << chan << endl;
    outputFile << "# " << pmt << endl;
    outputFile << "# Reference window: " << this->refWindow << endl;
    outputFile << "# Windows: ";
    for(size_t window = 0; window < this->windows.size(); window++)
    {
        outputFile << this->windows[window] << " ";
    }
    outputFile << endl;
    outputFile << "# Time\tVoltage" << endl;
    cout.precision(5);
    for(size_t i = 0; i < this->voltages.size(); i++)
    {
        outputFile << times[i] << "\t" << voltages[i] << endl;
    }
    outputFile.close();
}

std::string Curve::exportToGnuplot()
{
    std::stringstream outputFile;
    Channel chan = Channel::decodeChannel(this->channel);
    outputFile << "# Event: " << this->event << endl;
    outputFile << "# Scrod: " << (int)this->scrod << "; " << chan << endl;
    outputFile << "# " << pmt << endl;
    outputFile << "# Reference window: " << this->refWindow << endl;
    outputFile << "# Windows: ";
    for(size_t window = 0; window < this->windows.size(); window++)
    {
        outputFile << this->windows[window] << " ";
    }
    outputFile << endl;
    outputFile << "# Time\tVoltage" << endl;
    cout.precision(5);
    for(size_t i = 0; i < this->voltages.size(); i++)
    {
        outputFile << times[i] << "\t" << voltages[i] << endl;
    }
    return outputFile.str();
}

}
}
