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

int cardNum(ScrodNet* net)
{
    try
    {
        return net->getCardInfo().size()*5;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int cardInfo(ScrodNet* net, int* ret)
{
    try
    {
        std::vector<CardInfo> infos = net->getCardInfo();
        for(int i = 0; i < infos.size(); i++)
        {
            ret[i*5] = infos[i].id;
            ret[i*5 + 1] = (bool)(infos[i].chan & 1);
            ret[i*5 + 2] = (bool)(infos[i].chan & (1 << 1));
            ret[i*5 + 3] = (bool)(infos[i].chan & (1 << 2));
            ret[i*5 + 4] = (bool)(infos[i].chan & (1 << 3));
        }
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
    return 0;
}

}

