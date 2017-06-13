
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <mtc/readoutd2/scrodnet.hpp>
#include "../lib/config.hpp"

using namespace std;
using mtc::net2::ScrodNet;

extern "C"
{
ScrodNet* createConnection()
{
    char *ip;
    char *portString;
    ip = getenv(ENV_IP);
    portString = getenv(ENV_PORT);
    if (ip == NULL )
        ip = DEFAULT_IP;
    if(portString == NULL)
	portString = DEFAULT_PORT;

    int port;
    try
    {
        port = boost::lexical_cast<int>(portString);
    }
    catch(boost::bad_lexical_cast &e)
    {
        return NULL;
    }
    try
    {
        ScrodNet *a = new ScrodNet(port,ip);
        return a;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return NULL;
    }
}

void closeConnection(ScrodNet* net)
{
    delete net;
}
}
