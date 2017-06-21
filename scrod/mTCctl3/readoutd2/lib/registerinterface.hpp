#ifndef REGISTERINTERFACE_HPP
#define REGISTERINTERFACE_HPP

#include "settings.hpp"
#include "storage.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <string>

class RegisterInterface
{
public:
    RegisterInterface( std::vector<int> interfaces, Storage* s);
    std::vector<int> getIDs();
    void start();
    void stop();
    int numClients;
private:

    typedef struct
    {
        int sock;
        bool ok;
        int id;
        int seq;
        int clientSock;
    } ScrodRegisterInterface;

    void mainLoop();
    void setupSockets();
    ScrodRegisterInterface *findInterface(int id);
    std::vector<int> interfaces;
    std::vector<ScrodRegisterInterface> scrods;
    fd_set serverFdSet;
    uint maxFd;

    struct RegisterClient
    {
        struct sockaddr_in addr;
        int fd;
        int udpFd;
        bool upgraded;
    };

    std::list<RegisterClient> clients;
    int requestFd;

    boost::thread thr;
    int pipefd[2];  //0 read; 1 write;

    //pointer to the storage thread;
    Storage* s;
};

#endif // REGISTERINTERFACE_HPP
