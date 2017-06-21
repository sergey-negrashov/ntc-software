#ifndef PMTFACEMAP_HPP
#define PMTFACEMAP_HPP

#include <map>
#include <stdint.h>
#include <stdexcept>

namespace mtc
{
namespace data
{

static const std::string PMT_TO_FACE_LOCATION_ON_THE_CUBE_FILE = "/usr/local/mtc/scrod/face.map";

///Pmt to cube side position.
typedef struct PmtFacePositionStruct
{
    int8_t face;
    int8_t position;
}PmtFacePosition;

std::ostream& operator<<(std::ostream& os, const PmtFacePosition& c);

/**
 * @brief The PmtPositionMap class is used for converting a pmt number to position on the cube.
 */
class PmtPositionMap
{
public:
    PmtPositionMap();
    void parse() throw (std::runtime_error);
    PmtFacePosition getPosition(uint8_t pmt);
private:
    bool ok_;
    std::map<uint8_t, PmtFacePosition> pmtMap_;
};
}
}

#endif // PMTFACEMAP_HPP
