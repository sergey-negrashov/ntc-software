#include "lib/udplistner.hpp"
#include "lib/common.hpp"
#include "lib/packets.hpp"
#include "lib/netutil.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <boost/thread.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

UdpDataListner::UdpDataListner(int interface, StorageQueuePtr outQ)
{
    this->interface = interface;
    sock = -1;
    this->outQ = outQ;

    MtcState * state = MtcState::Instance();
    delim = boost::lexical_cast<string>(interface);
    ip = state->getValue(LISTNER_PREFACE + delim + SETTING_IP_LOCAL);
    port = boost::lexical_cast<uint>(state->getValue(LISTNER_PREFACE + SETTING_PORT));
    interName = ipToInterface(ip);
    setupState();
}


void UdpDataListner::setupState()
{
    //Statistics
    packetsRead = 0;
    bytesRead = 0;
    badPackets = 0;

    MtcState * state = MtcState::Instance();
    state->setValue(LISTNER_PREFACE + delim + LISTNER_STATE, "0");

    state->setValue(LISTNER_PREFACE + delim + LISTNER_TOTAL_PACKETS, "0");
    state->setValue(LISTNER_PREFACE + delim + LISTNER_TOTAL_BYTES, "0");
    state->setValue(LISTNER_PREFACE + delim + LISTNER_BAD_PACKETS, "0");

    bool up = isInterfaceUp(interName);
    if(up) state->setValue(LISTNER_PREFACE + delim + LISTNER_CONNECTED, "1");
    else state->setValue(LISTNER_PREFACE + delim + LISTNER_CONNECTED, "0");
}

void UdpDataListner::updateState(bool updateInterface)
{
    MtcState * state = MtcState::Instance();
    if(updateInterface)
    {
        bool up = isInterfaceUp(interName);
        if(up)
            state->setValue(LISTNER_PREFACE + delim + LISTNER_CONNECTED, "1");
        else
            state->setValue(LISTNER_PREFACE + delim + LISTNER_CONNECTED, "0");
    }
    state->setValue(LISTNER_PREFACE + delim + LISTNER_TOTAL_PACKETS, boost::str(boost::format("%d") % packetsRead));
    state->setValue(LISTNER_PREFACE + delim + LISTNER_TOTAL_BYTES, boost::str(boost::format("%0.4f") % (bytesRead/1024/1024.0)));
    state->setValue(LISTNER_PREFACE + delim + LISTNER_BAD_PACKETS, boost::str(boost::format("%d") % badPackets));
}


void UdpDataListner::start()
{
    if(thr.joinable())
    {
        throw std::runtime_error("Listner " + delim + ": attempting to start but already running");
    }
    //Setup pipe
    if (pipe(pipefd) == -1)
    {
        throw std::runtime_error("Listner " + delim + ": Could not create pipe");
    }
    thr = boost::thread(&UdpDataListner::mainLoop, this);

}
void UdpDataListner::stop()
{
    if(!thr.joinable())
    {
        throw std::runtime_error("Listner " + delim + ": attempting to stop but already done");
    }
    char a;
    ::write(pipefd[1], &a, sizeof(char));
    thr.interrupt();
    thr.join();
    close(sock);
    close(pipefd[0]);
    close(pipefd[1]);
}

void UdpDataListner::setupSocket()
{
    close(sock);
    //Set up socket;
    struct sockaddr_in addr;
    struct hostent *hp;

    MtcState * state = MtcState::Instance();
    memset((char *)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(ip.c_str(), (in_addr*)&addr.sin_addr.s_addr);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        throw std::runtime_error("Listner " + delim + ": Could not bind to " + ip);
    }
}

void UdpDataListner::mainLoop()
{
    //get a socket up
    try
    {
        setupSocket();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        cout << "Listner "<<interface <<": Done" << endl;
        return;
    }

    //setup select
    fd_set rfds, activeFdSet;
    struct timeval tv;
    int retval;
    FD_ZERO(&activeFdSet);
    FD_SET(pipefd[0],&activeFdSet);
    FD_SET(sock, &activeFdSet);
    int maxFd;
    if(pipefd[0] > sock) maxFd = pipefd[0] +1 ;
    else maxFd = sock +1;

    int loopCounter = 0;


    try
    {
        while(true)
        {
            boost::this_thread::interruption_point();
            rfds = activeFdSet;
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            retval = ::select(maxFd+1, &rfds, NULL, NULL, &tv);
            if (retval == 0)
            {
                //We slept for 1 second with no data. Better check if the interface is still up.
                updateState(true);
            }
            else
            {
                if(FD_ISSET(sock, &rfds))
                {
                    int size;
                    ioctl(sock, FIONREAD, &size);
                    try
                    {
                        while(size > 0)
                        {

                            uint32_t* p = readPacketHeap(sock, size, CHECK_PACKET_CHECKSUM);
                            packetsRead++;
                            bytesRead += size;
                            outQ->push(p);
                            ioctl(sock, FIONREAD, &size);
                            if(loopCounter %1000 == 0)
                            {
                                boost::this_thread::interruption_point();
                                updateState(false);
                            }
                            loopCounter++;
                            //cout << "Packet" << endl;
                        }
                    }
                    catch(std::runtime_error &e)
                    {
                        cerr << e.what() << endl;
                        badPackets++;
                        //cout << badPackets << endl;
                        //We got a bad packet. Better luck next time
                    }
                }
                if(FD_ISSET(pipefd[0], &rfds))
                {
                    char temp;
                    ::read(pipefd[0], &temp, 1);
                    continue;
                }
            }
        }
    }
    catch(boost::thread_interrupted &e)
    {
        //We have been interupted from the main thread;
    }
    cout << "Listner "<<interface <<": Done" << endl;
    FD_ZERO(&activeFdSet);
    return;
}


