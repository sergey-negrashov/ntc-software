#include "../lib/scrodmap.hpp"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

using namespace mtc::data;
using namespace std;

std::ostream& mtc::data::operator<<(std::ostream& os, const ScrodPosition& c)
{
    if(c.face == -1)
        os << "Face: Unknown";
    else
        os << "Face: " << (int)c.face;
    if(c.inverted)
        os << " Inverted: True";
    else
        os << " Inverted: False";
    return os;
}


ScrodPositionMap::ScrodPositionMap()
{
    ok_ = false;
}

void ScrodPositionMap::parse() throw (std::runtime_error)
{
    ifstream configFile(SCROD_LOCATION_ON_THE_CUBE_FILE.c_str(), ios::in);
    if(!configFile.is_open())
        throw runtime_error("Could not open scrod map master list at " + SCROD_LOCATION_ON_THE_CUBE_FILE);
    int lineCounter = 0;
    for( std::string line; getline( configFile, line ); )
    {
        lineCounter++;
        int face;
        int scrodUp;
        int scrodDown;
        boost::algorithm::trim(line);
        if(line.length() == 0)
            continue;
        if(line.at(0) == '#')
            continue;
        std::istringstream iss(line);
        if (!(iss >> face))
            throw std::runtime_error("Format error on line " + boost::lexical_cast<std::string>(lineCounter) + " file " + SCROD_LOCATION_ON_THE_CUBE_FILE);

        if (!(iss >> scrodUp))
            throw std::runtime_error("Format error on line " + boost::lexical_cast<std::string>(lineCounter) + " file " + SCROD_LOCATION_ON_THE_CUBE_FILE);

        if (!(iss >> scrodDown))
            throw std::runtime_error("Format error on line " + boost::lexical_cast<std::string>(lineCounter) + " file " + SCROD_LOCATION_ON_THE_CUBE_FILE);
        ScrodPosition p;
        p.face = face;
        p.inverted = false;
        scrodMap_.insert(std::make_pair(scrodUp, p));
        p.inverted = true;
        scrodMap_.insert(std::make_pair(scrodDown, p));
    }
    ok_ = true;
}

ScrodPosition ScrodPositionMap::getPosition(uint8_t scrodId)
{
    ScrodPosition bad;
    bad.face = -1;
    bad.inverted = false;
    if(!ok_)
        return bad;
    if(scrodMap_.find(scrodId) == scrodMap_.end())
        return bad;
    return scrodMap_[scrodId];
}
