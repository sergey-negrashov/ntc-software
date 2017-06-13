#include "../lib/scrodnet.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <bitset>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <mtc/readoutd2/readoutProtocol.hpp>
#include <boost/date_time.hpp>
#include <boost/thread.hpp>
#include <sys/ioctl.h>

using namespace mtc::net2;

static const uint32_t PACKET_HEADER =  0x00BE11E2;

static inline uint32_t checksum(uint32_t *packet, int size)
{
    uint32_t sum = 0;
    for(int i = 0; i< size; i++)
    {
        sum += packet[i];
    }
    return sum;
}
static inline bool checkChecksum(uint32_t *packet, int size)
{
    return checksum(packet, size - 1) == packet[size - 1];
}


u_int16_t doOp(int sock, OPERRATION op, uint16_t data = 0, uint16_t address = 0, uint8_t card = 0, uint8_t chan = 0);

ScrodNet::ScrodNet(int port, std::string ip) throw(std::runtime_error)
{
    port_ = port;
    ip_ =  ip;
    struct sockaddr_in serv_addr;
    if((sock_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ::close(sock_);
        throw std::runtime_error("Could not create a socket");
    }
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt (sock_, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        ::close(sock_);
        throw std::runtime_error("Could not create a socket");
    }

    if (setsockopt (sock_, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        ::close(sock_);
        throw std::runtime_error("Could not create a socket");
    }


    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0)
    {
        ::close(sock_);
        throw std::runtime_error("Could not resolve" + ip);
    }
    if( connect(sock_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        ::close(sock_);
        throw std::runtime_error("Could not connect to " + ip);
    }
    lastError = -1;
    upgraded_ = false;
}

ScrodNet::~ScrodNet()
{
    ::close(sock_);
    ::close(udpSock_);
}

std::vector<CardInfo> ScrodNet::getCardInfo()
{
    std::vector<CardInfo> ret;
    DspInfoHeader info;
    DspCardInfo temp;
    DspCommand cmd;
    cmd.op = GET_INFO;
    cmd.address = 0;
    cmd.arg = 0;
    cmd.chan = 0;
    cmd.card = 0;
    if(::write(sock_, &cmd, sizeof(DspCommand)) <= 0)
        return ret;
    if(::read(sock_, &info, sizeof(DspInfoHeader)) <= 0)
        return ret;
    for(int i = 0; i< info.cardNum; i++)
    {
        ::read(sock_, &temp, sizeof(DspCardInfo));
        CardInfo c;
        c.chan = temp.chan;
        c.id = temp.id;
        c.ioIn = temp.ioIn;
        c.ioOut = temp.ioOut;
        ret.push_back(c);
    }
    return ret;
}

uint16_t ScrodNet::getReg(uint16_t card, uint16_t channel, uint16_t address) throw (std::runtime_error)
{
    return doOp(sock_, GET_REG, 0, address, card, channel);
}

void ScrodNet::setReg(uint8_t card, uint8_t channel, uint16_t address, uint16_t data) throw (std::runtime_error)
{
    doOp(sock_, SET_REG, data, address, card, channel);
}

void ScrodNet::setTriggerMask(uint16_t mask)
{
    doOp(sock_, TRG_MASK_SET, mask);
}

uint16_t ScrodNet::getTriggerMask()
{
    return doOp(sock_, TRG_MASK_GET);
}

uint8_t ScrodNet::setTriggerMin(uint8_t min)
{
    return doOp(sock_, TRG_MIN_SET, min);
}

uint8_t ScrodNet::getTriggerMin()
{
    return doOp(sock_, TRG_MIN_GET);
}

uint ScrodNet::getTriggerCount()
{
    return doOp(sock_, TRG_COUNT_GET);
}

void ScrodNet::softTrigger()
{
    doOp(sock_, TRG_SOFT);
}

void ScrodNet::enableVeto(bool on)
{
    doOp(sock_, TRG_VETO_EN_SET, on);
}

bool ScrodNet::getVetoEnabled()
{
    return doOp(sock_, TRG_VETO_EN_GET);
}

bool ScrodNet::getVeto()
{
    return doOp(sock_, TRG_VETO_GET);
}

void ScrodNet::clearVeto()
{
    doOp(sock_, TRG_VETO_CLEAR);
}

void ScrodNet::setDelay(uint16_t delay)
{
    doOp(sock_, CAL_DELAY_SET, delay);
}

uint16_t ScrodNet::getDelay()
{
    return doOp(sock_, CAL_DELAY_GET);
}

void ScrodNet::enableCal(bool on)
{
    doOp(sock_, CAL_EN_SET, on);
}

bool ScrodNet::getCal()
{
    return doOp(sock_, CAL_EN_GET);
}

void ScrodNet::setTriggerDelay(uint8_t delay)
{
    doOp(sock_, CAL_TRG_DELAY_SET, delay);
}

uint8_t ScrodNet::getTriggerDelay()
{
    return doOp(sock_, CAL_TRG_DELAY_GET);
}

void ScrodNet::setCoarseDelay(uint8_t delay)
{
    doOp(sock_, CAL_COARSE_DELAY_SET, delay);
}

uint8_t ScrodNet::getCoarseDelay()
{
    return doOp(sock_, CAL_COARSE_DELAY_GET);
}

void ScrodNet::rotateStorage()
{
    doOp(sock_, STORAGE_ROTATE);
}

uint16_t doOp(int sock, OPERRATION op, uint16_t data, uint16_t address, uint8_t card, uint8_t chan)
{
    DspCommand cmd;
    DspResponce rsp;
    cmd.op = op;
    cmd.address = address;
    cmd.arg = data;
    cmd.card = card;
    cmd.chan = chan;
    if(::write(sock, &cmd, sizeof(DspCommand)) <= 0)
    {
        throw std::runtime_error("Could not write to socket");
    }
    // Original version
//    if(::read(sock, &rsp, sizeof(DspResponce)) <= 0)
//    {
//        throw std::runtime_error("Could not read from socket");
//    }
    // Kurtis' attempt at a patch:
    // Add a loop to read all available data, only verify against the last
    // DspResponce seen (another option would be to add a sequence number
    // to DspResponces, which would allow the client to detect when sync
    // is lost and try again under those conditions).
    ssize_t totalBytesRead = 0;
    ssize_t bytesRead = 0;
    ssize_t bytesRemaining = 0;
    do {
       bytesRead = ::read(sock, &rsp, sizeof(DspResponce));
       totalBytesRead += bytesRead;
       ioctl(sock, FIONREAD, &bytesRemaining);
       if (bytesRemaining > 0) {
           std::cerr << "Looks like " << bytesRemaining << " bytes were left to read!" << std::endl;
       }
    } while (bytesRemaining > 0);
    if(totalBytesRead <= 0)
    {
        throw std::runtime_error("Could not read from socket");
    }
    if (totalBytesRead > (ssize_t) sizeof(DspResponce)) {
        std::cerr << "Caught a stale DspResponce." << std::endl;
    }
    if(rsp.ok)
        return rsp.arg;
    throw std::runtime_error("Error: " + std::bitset<8>(rsp.arg).to_string());

}

void ScrodNet::upgrade(std::vector<mtc::data::GlobalChannel> channels) throw (std::runtime_error)
{
    std::map <uint, DspSubscription> subscription;
    for(size_t i = 0; i< channels.size(); i++)
    {
        int scrod = channels[i].scrod;
        mtc::data::Channel chan = mtc::data::Channel::decodeChannel(channels[i].channel);
        if(subscription.find(scrod) == subscription.end())
        {
            DspSubscription sub;
            sub.scrod = scrod;
            sub.row0 = 0;
            sub.row1 = 0;
            sub.row2 = 0;
            sub.row3 = 0;
            subscription[scrod] = sub;
        }
        if(chan.row == 0)
            subscription[scrod].row0 |= 1 << (chan.col*8 + chan.chan);
        if(chan.row == 1)
            subscription[scrod].row1 |= 1 << (chan.col*8 + chan.chan);
        if(chan.row == 2)
            subscription[scrod].row2 |= 1 << (chan.col*8 + chan.chan);
        if(chan.row == 3)
            subscription[scrod].row3 |= 1 << (chan.col*8 + chan.chan);
    }
    DspCommand cmd;
    cmd.op = STORAGE_UPGRADE;
    cmd.address = 0;
    cmd.card = 0;
    cmd.chan = 0;
    cmd.arg = subscription.size();
    if(::write(sock_, &cmd, sizeof(DspCommand)) <= 0)
    {
        lastError = -1;
        throw std::runtime_error("Could not write socket");
    }
    std::map <uint, DspSubscription>::iterator it;
    for(it = subscription.begin(); it != subscription.end(); it++)
    {
        if(::write(sock_,&(it->second), sizeof(DspSubscription)) <= 0)
        {
            lastError = -1;
            throw std::runtime_error("Could not write to socket");
        }
    }
    upgraded_ = true;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    udpSock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(udpSock_, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("Could not bind during upgrade");
}

uint32_t* ScrodNet::readPacket(uint32_t usecTimeout) throw (std::runtime_error)
{
    if(!upgraded_)
        throw std::runtime_error("Socket must be upgraded to a storage socket");

    int res;
    fd_set readable;

    FD_ZERO(&readable);
    FD_SET(udpSock_, &readable);

    if(usecTimeout >=0)
    {
        timeval  timeout;
        timeout.tv_sec = usecTimeout/1000000;
        timeout.tv_usec = usecTimeout%1000000;
        res = select(udpSock_+1,&readable,NULL,NULL,&timeout);
    }
    else
        res = select(udpSock_+1,&readable,NULL,NULL,NULL);

    if(res < 1)
        return NULL;

    int size;
    if(ioctl(udpSock_, FIONREAD, &size) < 0)
        return NULL;

    uint32_t* buffer = new uint32_t[size/4];

    if (::read(udpSock_, buffer, size) < 0)
    {
        delete buffer;
        return NULL;
    }
    return buffer;
}

bool ScrodNet::isSocketReady()
{
    bool    res;
    fd_set  readable;
    timeval  now;

    FD_ZERO(&readable);
    FD_SET(udpSock_, &readable);

    now.tv_sec = 0;
    now.tv_usec = 0;

    res = select(udpSock_+1,&readable,NULL,NULL,&now);

    if((res > 0) && FD_ISSET(udpSock_,&readable))
        return true;

    return false;
}

int ScrodNet::getSocket()
{
    if(!upgraded_)
        return -1;
    return udpSock_;
}


uint8_t ScrodNet::getEnable() {return doOp(sock_, GET_TRG_EN);}
void ScrodNet::setEnable(uint8_t val) {doOp(sock_, SET_TRG_EN, val);}

uint16_t ScrodNet::getMinA() {return doOp(sock_, GET_MIN_A);}
void ScrodNet::setMinA(uint16_t val) {doOp(sock_, SET_MIN_A, val);}

uint16_t ScrodNet::getMaxA() {return doOp(sock_, GET_MAX_A);}
void ScrodNet::setMaxA(uint16_t val) {doOp(sock_, SET_MAX_A, val);}

uint16_t ScrodNet::getMinB() {return doOp(sock_, GET_MIN_B);}
void ScrodNet::setMinB(uint16_t val) {doOp(sock_, SET_MIN_B, val);}

uint16_t ScrodNet::getMaxB() {return doOp(sock_, GET_MAX_B);}
void ScrodNet::setMaxB(uint16_t val) {doOp(sock_, SET_MAX_B, val);}

uint16_t ScrodNet::getMinDelayAB() {return doOp(sock_, GET_MIN_DELAY_AB);}
void ScrodNet::setMinDelayAB(uint16_t val) {doOp(sock_, SET_MIN_DELAY_AB, val);}

uint16_t ScrodNet::getMaxDelayAB() {return doOp(sock_, GET_MAX_DELAY_AB);}
void ScrodNet::setMaxDelayAB(uint16_t val) {doOp(sock_, SET_MAX_DELAY_AB, val);}

uint16_t ScrodNet::getMinC() {return doOp(sock_, GET_MIN_C);}
void ScrodNet::setMinC(uint16_t val) {doOp(sock_, SET_MIN_C, val);}

uint16_t ScrodNet::getMaxC() {return doOp(sock_, GET_MAX_C);}
void ScrodNet::setMaxC(uint16_t val) {doOp(sock_, SET_MAX_C, val);}

uint8_t ScrodNet::getPrescaleA() {return doOp(sock_, GET_PRESCALE_A);}
void ScrodNet::setPrescaleA(uint8_t val) {doOp(sock_, SET_PRESCALE_A, val);}

uint8_t ScrodNet::getPrescaleB() {return doOp(sock_, GET_PRESCALE_B);}
void ScrodNet::setPrescaleB(uint8_t val) {doOp(sock_, SET_PRESCALE_B, val);}

uint8_t ScrodNet::getPrescaleC() {return doOp(sock_, GET_PRESCALE_C);}
void ScrodNet::setPrescaleC(uint8_t val) {doOp(sock_, SET_PRESCALE_C, val);}

uint8_t ScrodNet::getPrescaleAB() {return doOp(sock_, GET_PRESCALE_AB);}
void ScrodNet::setPrescaleAB(uint8_t val) {doOp(sock_, SET_PRESCALE_AB, val);}

uint16_t ScrodNet::getTrgLinkStatus() {return doOp(sock_, GET_TRG_LINK_STATUS);}

uint8_t ScrodNet::getLiveTimeFraction() {return doOp(sock_, GET_LIVE_TIME);}

uint16_t ScrodNet::getTriggerARate() {return doOp(sock_, GET_TRG_A_RATE);}
uint16_t ScrodNet::getTriggerBRate() {return doOp(sock_, GET_TRG_B_RATE);}
uint16_t ScrodNet::getTriggerCRate() {return doOp(sock_, GET_TRG_C_RATE);}
uint16_t ScrodNet::getTriggerABRate() {return doOp(sock_, GET_TRG_AB_RATE);}
