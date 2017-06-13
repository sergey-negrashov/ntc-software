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
#include <mtc/data/util.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
using mtc::net2::ScrodNet;
using mtc::net2::CardInfo;

using namespace mtc::data;

extern "C"
{

int rotateStorageFile(ScrodNet* net)
{
    try
    {
        net->rotateStorage();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
    return 0;
}

int collectEvents(int eventMax, char* fname, int timeout)
{
    ScrodNet* net;
    int fd = ::open(fname, O_CREAT | O_WRONLY,0666);
    if(fd < 0)
    {
        cout << "Could not open file for output " << fname << endl;
        return -1;
    }

    try
    {
        net = createConnection();
        net->upgrade();
    }
    catch(std::runtime_error &e)
    {
        delete net;
        return -1;
    }

    int currentEvent = 0;
    int eventCounter = 0;

    while(true)
    {
        uint32_t* packetData;
        try
        {
            packetData = net->readPacket(timeout);
        }
        catch(std::runtime_error &e)
        {
            continue;
        }

        if(packetData == NULL)
            break;

        if(packetData[2] == mtc::data::DataPacket::EVENT_WORD)
        {
            int event = packetData[5];
            if(currentEvent < event)
            {
                currentEvent = event;
                eventCounter++;
            }
            if(eventCounter > eventMax)
                break;
        }
        ::write(fd, packetData, (packetData[1] + 2)*sizeof(uint32_t));
        delete[] packetData;
    }
    delete net;
    ::close(fd);
    return eventCounter;
}

ScrodNet* openMonitor(bool specifyChannels, 
                      uint16_t scrod = 0, 
                      uint8_t channelStart = 0, uint8_t channelStop = 0) {

   ScrodNet* net;
   try {
      net = createConnection();
      if (specifyChannels) {
         vector<GlobalChannelStruct> selectedChannels;
         for (uint8_t i = channelStart; i <= channelStop; ++i) {
            GlobalChannelStruct thisChannel;
            thisChannel.scrod = scrod;
            thisChannel.channel = i;
            selectedChannels.push_back(thisChannel);
            cout << "Added to subscription, SCROD: " << int(scrod) << " CH: " << int(i) << endl;
         }
         net->upgrade(selectedChannels);
      } else {
         net->upgrade();
      }
   } catch(std::runtime_error &e) {
      delete net;
      net = NULL;
   }
   return net;

}

uint32_t* liveCollectPacket(ScrodNet* net, int timeout, int &length) {

   if (net == NULL) {
      net = openMonitor(false);
   }

   uint32_t* packetData = NULL;
   if (net) {
      try {
         packetData = net->readPacket(timeout);
      } catch(std::runtime_error &e) {
         packetData = NULL;
      }
   }

   if (packetData) {
      length = packetData[1] + 1;
   } else {
      length = 0;
      packetData = new uint32_t[1];
   }

   return packetData;
}

}
