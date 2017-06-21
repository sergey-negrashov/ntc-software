#ifndef COMMANDLINE_HPP
#define COMMANDLINE_HPP

#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <time.h>
#include <bitset>
#include <mtc/readoutd2/scrodnet.hpp>

/**
 * @brief The CommandLine class controls the readout system locally.
 *It is the parrent of all of the other classes.
 */
class CommandLine
{
public:

    /**
     * Default constructor.
     */
    CommandLine(std::string ip);

    /**
     * @brief mainLoop The main command loop.
     */
    void mainLoop();
private:

    bool processCommand(std::string);

    uint card_;
    uint chan_;
    bool ready_;
    boost::shared_ptr<mtc::net2::ScrodNet> net_;
    enum BASE {HEX, DEC, BIN};
    BASE base_;
    std::string ip_;
};

#endif // COMMANDLINE_HPP
