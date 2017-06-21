#ifndef SCRODMAP_HPP
#define SCRODMAP_HPP
#include <map>
#include <stdint.h>
#include <stdexcept>

namespace mtc
{
namespace data
{

static const std::string SCROD_LOCATION_ON_THE_CUBE_FILE = "/usr/local/mtc/scrod/scrod.map";

//Scrod position on the cube.
typedef struct ScrodPositionStruct
{
    int8_t face;
    bool inverted;
}ScrodPosition;

std::ostream& operator<<(std::ostream& os, const ScrodPosition& c);

/**
 * @brief The ScrodPositionMap class is used for finding the scrod position on the cube.
 */
class ScrodPositionMap
{
public:
    ScrodPositionMap();
    void parse() throw (std::runtime_error);
    ScrodPosition getPosition(uint8_t scrodId);
private:
    bool ok_;
    std::map<uint8_t, ScrodPosition> scrodMap_;
};
}
}
#endif // SCRODMAP_HPP
