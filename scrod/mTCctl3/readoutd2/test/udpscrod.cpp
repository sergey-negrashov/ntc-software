#include "lib/packets.hpp"
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#include <sys/ioctl.h>
#include <net/if.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <boost/algorithm/string.hpp>
#include <map>

using namespace std;

int sock;
int sequence;

int setupSocket(std::string ip, uint port)
{
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    struct sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    inet_aton(ip.c_str(), (in_addr*)&localAddr.sin_addr.s_addr);
    localAddr.sin_port=htons(port);

    sock = ::socket(AF_INET,SOCK_DGRAM,0);
    ::bind(sock, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if(sock < 0)
        throw std::runtime_error("could not connect to " + ip);
}

int main(int argc, char** argv)
{
    if(argc != 3) return 0;
    uint port = boost::lexical_cast<uint>(argv[2]);
    string ip = argv[1];
    setupSocket(ip, port);
    socklen_t len;
    char bufin[1000];
    struct sockaddr_in remote;

    len = sizeof(remote);
    map <uint16_t, uint16_t> regs;
    while (1)
    {
        /* read a datagram from the socket (put result in bufin) */
        recvfrom(sock,bufin,1000,0,(struct sockaddr *)&remote,&len);
        RegisterCommand *c = (RegisterCommand*)bufin;
        uint16_t address = c->valueAddress & 0xFFFF;
        uint16_t arg = (c->valueAddress >> 16) & 0xFFFF;

        RegisterResponce r;
        r.belleHeader = PACKET_HEADER;
        r.size = sizeof(r)/4 -2;
        r.packetType = CMD_RESPONSE_SUCCESS_WORD;
        r.dest = 0;
        r.id = 0;
        r.command  = c->registerCommand;
        r.valueAddress = c->valueAddress;
        if(c->registerCommand == CMD_WRITE_REGISTER_WORD)
            regs[address] = arg;
        else
        {
            r.valueAddress = (uint32_t)(regs[address] << 16) | address;
        }
        r.checksum = r.belleHeader + r.size + r.packetType + r.dest + r.id + r.command + r.valueAddress;
        sendto(sock,&r,sizeof(r),0,(struct sockaddr *)&remote,len);
        cout << "packet" << endl;
    }
}

