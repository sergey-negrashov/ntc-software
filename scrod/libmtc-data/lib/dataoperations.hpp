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

#ifndef DATAOPERATIONS_HPP
#define DATAOPERATIONS_HPP
#include "index.hpp"
#include "memory.hpp"
#include "datastore.hpp"
#include "channelignoremask.hpp"
#include "curve.hpp"

#include <vector>
#include <stdexcept>

namespace mtc
{
namespace data
{

Memory averageSamples(std::vector<DataStore*> &datas);
/**
 * @brief averageSamples average samples across a bunch of pedestals.
 * @param datas pedestals.
 * @return Resulting pedestal loaded in memory.
 */

std::vector<Pulse> findPulsesFromChannelNoTiming(Curve &c, uint minWidth = 5,int minHeight = 150, float threshold = 0.4, float step = 0.366) throw(std::runtime_error);
/**
 * @brief findPulsesFromChannelNoTiming Find all pulses in the curve and perform CFD
 * @param c Curve object.
 * @param minWidth Minimum Width cut.
 * @param minHeight Minimum Height cut.
 * @param threshold CFD threshold. Nominaly 40%.
 * @param step  Nominal step. 366ps.
 * @return vector of found pulses.
 */
//Min width is in time bins. Min height is in adc counts. threshold is CFD fraction


Curve generateCurveNoTiming(DataStore &data, int32_t event, uint8_t scrod, int8_t channel, PmtMap map = PmtMap(), float step = 0.366) throw(std::runtime_error);
//std::vector<Curve> generateCurvesNoTiming(DataStore &data, int32_t event, uint8_t scrod, int8_t channel, PmtMap map = PmtMap(), float step = 0.366) throw(std::runtime_error);
/**
 * @brief generateCurveNoTiming Generates a curve object for a specific event/scrod channel.
 * @param data Data store.
 * @param event Event number.
 * @param scrod Scrod ID.
 * @param channel Scrod Channel.
 * @param map Pmt map object for scrod to pmt mapping. Not required.
 * @param step Nominal step size. 366ps.
 * @return A curve object.
 */

Memory pruneChannels(DataStore &data, ChannelIgnoreMask& mask);
/**
 * @brief pruneChannels Prune Channel according to the ChannelIgnoreMask object.
 * @param data Data store.
 * @param mask Ignore mask.
 * @return Memory Pruned data all cached in memory.
 */


Memory subtractPedistal(DataStore &data, DataStore &ped);
Memory subtractPedistal(Memory &data, DataStore &ped); //Overloaded copy to allow for DataStore OR Memory type objects
/**
 * @brief subtractPedistal subtract pedistals
 * @param data Data store.
 * @param ped Pedestal store.
 * @return Ped subtracted data, all loaded in memory.
 */

bool skimEventCopy(Index &data, uint32_t event); //Used to determine whether a particular event is muonlike or not and should be skimmed from text conversions (.glenn, .dst)
/**
 * @brief skimEvent skims out non-neutrino like events from ab-neutrino data and is used in conjunction with scrod2glenn and scrod2dst.
 * @param data Data store.
 * @param uint32_t event.
 */


Memory trimBadEvents(DataStore &data);
/**
 * @brief pruneBadEvents removes known bad events from irs3d data
 * @param data Data store.
*/


}
}
#endif // DATAOPERATIONS_HPP
