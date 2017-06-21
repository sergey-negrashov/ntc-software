
#ifndef UDPLISTNER_HPP
#define UDPLISTNER_HPP
#include "settings.hpp"
#include <string>
#include <boost/thread.hpp>
#include "lib/common.hpp"
#include <chrono>

class UdpDataListner
{
public:
    UdpDataListner(int interface, StorageQueuePtr outQ);
    void start();
    void stop();

    uint64_t packetsRead;
    uint64_t bytesRead;
    uint64_t badPackets;

    std::string ip;
    std::string interName;
    std::string delim;

private:    
    boost::thread thr;
    void mainLoop();
    void setupState();
    void updateState(bool updateInterface = false);
    void setupSocket();

    uint32_t eventNumber;

    int interface;
    int sock;
    int pipefd[2];  //0 read; 1 write;
    uint port;

    StorageQueuePtr outQ;
};

#endif // UDPLISTNER_HPP
