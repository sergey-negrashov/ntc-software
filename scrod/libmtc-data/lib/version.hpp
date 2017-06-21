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

#ifndef VERSION_HPP
#define VERSION_HPP
#include <string>
#include <iostream>

namespace mtc
{
namespace data
{

#define LIB_MTC_DATA_VERSION "1.5"

std::string version();

/**
 * @brief checkVersion Dirty header trick for checking the version.
 * @return true if all is well. False if version is missmatched.
 */
bool checkVersion()
{
    if(version() != LIB_MTC_DATA_VERSION)
    {
        std::cerr << "WARNING!!! There is a missmatch between the libmtc-data library installed on this system and the one that your application was linked against." << std::endl;
        std::cerr << "Library version " << version() << std::endl;
        std::cerr << "Application version " << LIB_MTC_DATA_VERSION << std::endl;
        return false;
    }
    return true;
}

}
}
#endif // VERSION_HPP
