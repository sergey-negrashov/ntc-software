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

#ifndef CHANNELIGNOREMASK_HPP
#define CHANNELIGNOREMASK_HPP
#include "util.hpp"
#include <vector>

namespace mtc
{
namespace data
{

/**
 * @brief IGNORE_LIST_PATH path to the prunning list.
 */
static const std::string IGNORE_LIST_PATH = "/usr/local/mtc/scrod/channel.ignore";

/**
 * @brief The ChannelIgnoreMask class is used for dealing with the prunning list.
 */
class ChannelIgnoreMask
{
public:
    /**
     * @brief Mask scrod/channel pair.
     */
    typedef std::pair<int, uint16_t> Mask;
    ChannelIgnoreMask();

    /**
     * @brief parse parses the local pruning list.
     */
    void parse() throw(std::runtime_error);

    /**
     * @brief maskList vector of "bad" channels.
     */
    std::vector <Mask> maskList;
};
}
}
#endif // CHANNELIGNOREMASK_HPP
