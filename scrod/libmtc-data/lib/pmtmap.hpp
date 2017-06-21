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

#ifndef FRONTBOARDMAP_HPP
#define FRONTBOARDMAP_HPP
#include <map>
#include <stdint.h>
#include <stdexcept>

#include "util.hpp"

namespace mtc
{
namespace data
{
static const std::string FRONT_BOARD_AND_PMT_LIST_PATH = "/usr/local/mtc/scrod/pmt.map";

///Index of a pmt pixel.
typedef struct PmtChannelStruct
{
    ///Row of the PMT.
    uint8_t row;
    ///Column of the PMT.
    uint8_t col;
    ///PMT ID.
    int32_t pmt;
}PmtChannel;

std::ostream& operator<<(std::ostream& os, const PmtChannel& c);

/**
 * @brief The PmtMap class used for conversion of the scrod channel to the cube pmt channel.
 */
class PmtMap
{
public:
    PmtMap();
    void parse() throw(std::runtime_error) ;
    PmtChannel getPmtChannel(GlobalChannel gc);

private:
    std::map <uint32_t, PmtChannelStruct> pmts;
    bool ok;
};

}
}
#endif // FRONTBOARDMAP_HPP
