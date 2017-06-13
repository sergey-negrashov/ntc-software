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
int setReg(ScrodNet* net, int card, int chan, int reg, int val)
{
    try
    {
        net->setReg(card,chan,reg,val);
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
    return 0;
}
}
