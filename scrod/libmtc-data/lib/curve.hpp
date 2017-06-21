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

#ifndef CURVE_HPP
#define CURVE_HPP
#include <vector>
#include <stdint.h>
#include <string>
#include "pmtmap.hpp"

namespace mtc
{
namespace data
{

/**
 * @brief The CurveStruct struct is used to represent an adc trace.
 */
typedef  struct CurveStruct
{
    ///Voltages in ADC counts.
    std::vector <int> voltages;
    ///Times in ns relative to the reference window.
    std::vector <float> times;
    ///Window IDs in this curve in the same order as they appear in voltages.
    std::vector <uint16_t> windows;
    ///Reference window.
    uint16_t refWindow;
	//Trigger bit - 1 if trigger on in windows around the curve
	uint8_t trgBit;
    ///Scrod ID.
    uint8_t scrod;
    ///Scord channel.
    uint8_t channel;
    ///Event ID.
    uint32_t event;
    ///PMT channel for this event.
    PmtChannel pmt;
    ///Export to gnuplotable, human readable file.
    void exportToGnuplot(std::string filename);
    ///Export to gnuplotable string.
    std::string exportToGnuplot();
} Curve;

}
}
#endif // CURVE_HPP
