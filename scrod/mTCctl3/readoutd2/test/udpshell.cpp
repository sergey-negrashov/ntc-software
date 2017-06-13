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
using namespace std;

int sock;
int sequence;

int setupSocket(std::string ip, uint port)
{
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    struct sockaddr_in remoteAddr;
    remoteAddr.sin_family = AF_INET;
    inet_aton(ip.c_str(), (in_addr*)&remoteAddr.sin_addr.s_addr);
    remoteAddr.sin_port=htons(port);

    sock = ::socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    ::setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout));
    setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout));
    ::connect(sock, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
    if(sock < 0)
        throw std::runtime_error("could not connect to " + ip);
}

void processCommand(std::string in)
{
    std::vector<std::string> args;
    char* end;
    boost::algorithm::split(args, in, boost::algorithm::is_any_of(" \t"), boost::algorithm::token_compress_on);
    if(args.size()  == 0)
        return;
    in = args[0];
    int reg;
    int data;
    if(in == "quit")
    {
        exit(0);
    }
    else if(in == "set")
    {
        int reg;
        int data;
        if(args.size() < 3)
        {
            cout << "Set command requires two numerical arguments: register and data" << endl;
            return;
        }
        reg = strtoul(args[1].c_str(), &end, 0);
        if(*end)
        {
            cout << "Set command requires two numerical arguments: register and data" << endl;
            return;
        }
        data = strtoul(args[2].c_str(), &end, 0);
        if(*end)
        {
            cout << "Set command requires two numerical arguments: register and data" << endl;
            return;
        }
        try
        {
            setRegister(sock, reg,data,sequence);
            sequence++;
            int counter  = 0;
            int count;
            while(counter < 100)
            {
                usleep(1000);
                ioctl(sock, FIONREAD, &count);
                counter++;
                if(count > 0)
                {
                    break;
                }
            }
            if(counter  >= 100)
            {
                cout << "Timeout" << endl;
                return;
            }
            std::vector<uint32_t> packet =  readPacket(sock);
            uint32_t type = packet[2];

            switch(type)
            {
            case CMD_RESPONSE_SUCCESS_WORD:
            {
                RegisterResponce* out = (RegisterResponce*)packet.data();
                uint addr = out->valueAddress & 0xFFFF;
                uint val = (out->valueAddress >> 16) & 0xFFFF;
                cout << addr << " : " << val << endl;
            }
                break;

            case CMD_RESPONSE_FAILED_WORD:
            {
                FailedResponce* out = (FailedResponce*)packet.data();
                cout << "Command failed. Flags:" <<( out->valueAddress & 0xFFFF )<< endl;
            }
                break;

            default:
            {
                cout << "Unknown packet type" << type << endl;
            }
            }
        }
        catch(std::runtime_error &e)
        {
            cout << e.what() << endl;
            return ;
        }
        return ;
    }
    else if(in == "get")
    {
        if(args.size() <2)
        {
            cout << "Get command requires a numerical argument (register) " << endl;
            return ;
        }
        reg = strtoul(args[1].c_str(), &end, 0);
        if(*end)
        {
            cout <<  "Get command requires a numerical argument (register) " << endl;
            return ;
        }
        try
        {
            getRegister(sock, reg, sequence);
            sequence++;
            int counter  = 0;
            int count;
            while(counter < 100)
            {
                usleep(1000);
                ioctl(sock, FIONREAD, &count);
                counter++;
                if(count > 0)
                {
break;
                }
            }
            if(counter  >= 100)
            {
                cout << "Timeout" << endl;
                return;
            }
            std::vector<uint32_t> packet =  readPacket(sock);
            uint32_t type = packet[2];

            switch(type)
            {
            case CMD_RESPONSE_SUCCESS_WORD:
            {
                RegisterResponce* out = (RegisterResponce*)packet.data();
                uint addr = out->valueAddress & 0xFFFF;
                uint val = (out->valueAddress >> 16) & 0xFFFF;
                cout << addr << " : " << val << endl;
            }
                break;

            case CMD_RESPONSE_FAILED_WORD:
            {
                FailedResponce* out = (FailedResponce*)packet.data();
                cout << "Command failed. Flags:" << (out->valueAddress & 0xFFFF) << endl;
            }
                break;

            default:
            {
                cout << "Unknown packet type" << type << endl;
            }
            }
        }
        catch(std::runtime_error &e)
        {
            cout << e.what() << endl;
            return ;
        }

        return ;
    }
    else
    {
        cout << "bad command" << endl;
        return ;
    }
}

int main(int argc, char** argv)
{
    if(argc != 3) return 0;
    uint port = boost::lexical_cast<uint>(argv[2]);
    string ip = argv[1];
    setupSocket(ip, port);
    char* input = NULL;
    sequence = 0;
    std::string prompt = ip + ">";
    for(;;)
    {
        if(input != NULL)
        {
            free(input);
            input = NULL;
        }
        input = readline(prompt.c_str());

        if (!input)
            continue;

        string in(input);
        boost::algorithm::trim(in);
        if(in.length() == 0 || in.at(0) == '#')
            continue;
        add_history(input);
        processCommand(in);
    }

    return 0;
}
