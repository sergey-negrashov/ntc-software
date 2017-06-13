
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

#include <iostream>
#include <mtc/data/datapacket.hpp>
#include  "../lib/packets.hpp"

using namespace mtc::data;
using namespace std;

int main(int argc, char **argv)
{
    int sock,n;
    struct sockaddr_in servaddr,cliaddr;

    if(argc != 2) return 0;
    int fd = ::open(argv[1], O_RDONLY);
    sock=socket(AF_INET,SOCK_DGRAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_aton("127.0.0.1", (in_addr*)&servaddr.sin_addr.s_addr);
    servaddr.sin_port=htons(2000);
    connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr));
    try
    {
        while(true)
        {
            DataPacket p(fd);
            if(p.isNull) break;
            uint32_t sum = PACKET_HEADER;
            usleep(100);
            std::vector<uint32_t> data(p.size + 5);
            data[0] = PACKET_HEADER;
            data[1] = p.size + 3;
            sum += data[1];
            data[2] = p.type;
            sum += data[2];
            data[3] = p.scrodId << 16;
            sum += data[3];
            for(int i = 0; i < p.size;i++)
            {
                data[4+i] = p.data[i];
                sum += p.data[i];
            }
            data[p.size+4] = sum;
            cout << ::write(sock, data.data(), 4*data.size()) << endl;

        }
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return 0;
    }
    close(sock);
    close(fd);
    return 0;
}
