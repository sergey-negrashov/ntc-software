#include "../lib/pmtfacemap.hpp"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

using namespace mtc::data;
using namespace std;

std::ostream& mtc::data::operator<<(std::ostream& os, const PmtFacePosition& c)
{
    if(c.face == -1)
        os << "Face: Unknown";
    else
        os << "Face: " << (int)c.face << " Position " << (int) c.position;
    return os;
}


PmtPositionMap::PmtPositionMap()
{
    ok_ = false;
}

void PmtPositionMap::parse() throw (std::runtime_error)
{
    ifstream configFile(PMT_TO_FACE_LOCATION_ON_THE_CUBE_FILE.c_str(), ios::in);
    if(!configFile.is_open())
        throw runtime_error("Could not open pmt to face map master list at " + PMT_TO_FACE_LOCATION_ON_THE_CUBE_FILE);
    int lineCounter = 0;
    for( std::string line; getline( configFile, line ); )
    {
        lineCounter++;
        int face;
        int pmt;
        boost::algorithm::trim(line);
        if(line.length() == 0)
            continue;
        if(line.at(0) == '#')
            continue;
        std::istringstream iss(line);
        if (!(iss >> face))
            throw std::runtime_error("Format error on line " + boost::lexical_cast<std::string>(lineCounter) + " file " + PMT_TO_FACE_LOCATION_ON_THE_CUBE_FILE);
        for(int position = 0; position< 4; position++)
        {
            if (!(iss >> pmt))
                throw std::runtime_error("Format error on line " + boost::lexical_cast<std::string>(lineCounter) + " file " + PMT_TO_FACE_LOCATION_ON_THE_CUBE_FILE);
            PmtFacePosition p;
            p.face = face;
            p.position = position;
            pmtMap_.insert(std::make_pair(pmt,p));
        }
    }
    ok_ = true;
}

PmtFacePosition PmtPositionMap::getPosition(uint8_t pmt)
{
    PmtFacePosition bad;
    bad.face = -1;
    bad.position = -1;
    if(!ok_)
        return bad;
    if(pmtMap_.find(pmt) == pmtMap_.end())
        return bad;
    return pmtMap_[pmt];
}
