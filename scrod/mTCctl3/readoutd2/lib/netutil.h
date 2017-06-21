#ifndef NETUTIL_H
#define NETUTIL_H
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <net/if.h>


inline string ipToInterface(string ip)
{
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1)
    {
        throw std::runtime_error("getifaddrs");
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
            continue;
        char thisIp[100];
        getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), (char*)thisIp,100, NULL, 0, NI_NUMERICHOST);
        if (thisIp == ip)
            return ifa->ifa_name;
    }
    freeifaddrs(ifaddr);
    return "";
}


inline bool isInterfaceUp(string interface)
{
    struct ifreq ifr;
    int sock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_IP);
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, interface.c_str());
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
    {
            perror("SIOCGIFFLAGS");
    }
    close(sock);
    return (ifr.ifr_flags & IFF_RUNNING);
}


#endif // NETUTIL_H
