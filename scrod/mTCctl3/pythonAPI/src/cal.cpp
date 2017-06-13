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
using mtc::net2::CardInfo;


extern "C"
{
int calDelayGet(ScrodNet* net)
{
    try
    {
        return net->getDelay();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int calDelaySet(ScrodNet* net, int val)
{
    try
    {
        net->setDelay(val);
        return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}
int calEnGet(ScrodNet* net)
{
    try
    {
        return net->getCal();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int calEnSet(ScrodNet* net, bool on)
{
    try
    {
        net->enableCal(on);
        return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int calTrgDelayGet(ScrodNet* net)
{
    try
    {
        return net->getTriggerDelay();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int calTrgDelaySet(ScrodNet* net, int delay)
{
    try
    {
        net->setTriggerDelay(delay);
        return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int calCoarseDelayGet(ScrodNet* net)
{
    try
    {
       return net->getCoarseDelay();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int calCoarseDelaySet(ScrodNet* net, int delay)
{
    try
    {
        net->setCoarseDelay(delay);
        return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}
}
