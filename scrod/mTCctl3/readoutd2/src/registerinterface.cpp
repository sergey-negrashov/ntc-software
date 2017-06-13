#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "lib/packets.hpp"
#include "lib/readoutProtocol.hpp"
#include "lib/trigger.hpp"
#include "lib/registerinterface.hpp"
#include "lib/common.hpp"
#include "lib/motor.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>



using namespace std;

RegisterInterface::RegisterInterface(std::vector<int> interfaces, Storage *s) {
    this->interfaces = interfaces;
    maxFd = 0;
    setupSockets();
    this->s = s;
    numClients = 0;
}

void RegisterInterface::setupSockets(void) {
    MtcState *state = MtcState::Instance();
    uint port = boost::lexical_cast<uint>(state->getValue(REGISTER_PREFACE + SETTING_PORT));
    FD_ZERO(&serverFdSet);

    for (size_t i = 0; i < interfaces.size(); i++) {
        bool ok = true;
        std::string delim = boost::lexical_cast<string>(interfaces[i]);

        std::string ipLocal = state->getValue(REGISTER_PREFACE + delim + SETTING_IP_LOCAL);
        std::string ipRemote = state->getValue(REGISTER_PREFACE + delim + SETTING_IP_REMOTE);

        int sock;
        struct sockaddr_in localAddr, remoteAddr;

        ::bzero(&localAddr, sizeof(localAddr));
        ::bzero(&remoteAddr, sizeof(remoteAddr));

        localAddr.sin_family = AF_INET;
        ::inet_aton(ipLocal.c_str(), (in_addr *) &localAddr.sin_addr.s_addr);
        localAddr.sin_port = htons(port);


        remoteAddr.sin_family = AF_INET;
        ::inet_aton(ipRemote.c_str(), (in_addr *) &remoteAddr.sin_addr.s_addr);
        remoteAddr.sin_port = htons(port);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        ok = (sock > 0);
        ok = ok && (::bind(sock, (struct sockaddr *) &localAddr, sizeof(localAddr)) >= 0);
        ok = ok && (::connect(sock, (struct sockaddr *) &remoteAddr, sizeof(remoteAddr)) >= 0);
        ok = ok && (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) >= 0);
        int yes = 1;
        ok = ok && (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) >= 0);
        if (!ok) {
            cout << "Register thread: could not bind to " << ipLocal << endl;
        }
        ScrodRegisterInterface r;
        r.ok = ok;
        r.sock = sock;
        r.id = interfaces[i];
        r.seq = 0;
        scrods.push_back(r);
    }
}

void RegisterInterface::start() {
    if (thr.joinable()) {
        throw std::runtime_error("Register thread: attempting to start but already running");
    }
    MtcState *state = MtcState::Instance();
    struct sockaddr_in addr;
    uint port = boost::lexical_cast<uint>(state->getValue(SETTING_PORT_CLIENT));

    requestFd = ::socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    ::setsockopt(requestFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    ::memset((char *) &addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    //::inet_aton(ip.c_str(), (in_addr*)&addr.sin_addr.s_addr);

    if (::bind(requestFd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        throw std::runtime_error("Register thread: could not bind");

    ::listen(requestFd, 5);

    //Setup pipe
    if (pipe(pipefd) == -1)
        throw std::runtime_error("Register thread: Could not create pipe");
    FD_ZERO(&serverFdSet);
    if (pipefd[0] > maxFd)
        maxFd = pipefd[0];
    if (requestFd > maxFd)
        maxFd = requestFd;

    FD_SET(pipefd[0], &serverFdSet);
    FD_SET(requestFd, &serverFdSet);
    thr = boost::thread(&RegisterInterface::mainLoop, this);
}

void RegisterInterface::stop() {
    if (!thr.joinable()) {
        throw std::runtime_error("Register thread: attempting to start but already done");
    }
    char a;
    ::write(pipefd[1], &a, sizeof(char));
    thr.interrupt();
    thr.join();
}

void RegisterInterface::mainLoop() {
    fd_set rfds;
    struct sockaddr_in client_addr;
    socklen_t size = sizeof(client_addr);
    MtcState *state = MtcState::Instance();
    Motor m = Motor();
    try {
        while (true) {
            rfds = serverFdSet;
            boost::this_thread::interruption_point();
            struct timeval tv;
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            int retval = ::select(maxFd + 1, &rfds, NULL, NULL, &tv);
            if (retval == -1)
                break;

            if (retval == 0)
                continue;


            if (FD_ISSET(pipefd[0], &rfds)) {
                char temp;
                ::read(pipefd[0], &temp, 1);
                throw boost::thread_interrupted();
            }
            if (FD_ISSET(requestFd, &rfds)) {
                int clientFd = ::accept(requestFd, (struct sockaddr *) &client_addr, &size);
                if (clientFd < 0) {
                    break;
                }
                RegisterClient client;
                client.addr = client_addr;
                client.fd = clientFd;
                client.upgraded = false;
                FD_SET(clientFd, &serverFdSet);
                clients.push_back(client);
                if (maxFd < clientFd)
                    maxFd = clientFd;
                numClients++;
                continue;
            }
            auto it = clients.begin();
            while (it != clients.end()) {
                int client = (*it).fd;
                if (!FD_ISSET(client, &rfds)) {
                    it++;
                    continue;
                }
                int packetSize = 0;
                ioctl(client, FIONREAD, &packetSize);

                if (packetSize == 0) {
                    if (it->upgraded) {
                        s->removeClient(it->udpFd);
                        ::close(it->udpFd);
                    }
                    it = clients.erase(it);
                    FD_CLR(client, &serverFdSet);
                    ::close(client);
                    numClients--;
                    continue;
                } else if (it->upgraded) {
                    //We should not be getting messages here;
                } else {
                    DspCommand c;
                    ::read(client, &c, sizeof(DspCommand));
                    try {
                        switch (c.op) {
                            case SET_REG:
                            case GET_REG: {
                                DspResponce clientData;
                                int id = c.card * 4 + c.chan;
                                RegisterInterface::ScrodRegisterInterface *scrod = findInterface(id);
                                if (c.op == GET_REG)
                                    clientData = getRegister(scrod->sock, c.address, scrod->seq);
                                else
                                    clientData = setRegister(scrod->sock, c.address, c.arg, scrod->seq);
                                scrod->seq++;
                                clientData.card = c.card;
                                clientData.chan = c.chan;
                                ::write(client, (char *) &clientData, sizeof(clientData));
                                break;
                            }
                            case GET_INFO: {
                                std::string scrodString = state->getValue(ACTIVE_SCRODS);

                                std::vector<std::string> scrodStrings;
                                boost::split(scrodStrings, scrodString, boost::is_any_of(","));
                                std::vector<int> activeScrods;
                                for (auto i = scrodStrings.begin(); i != scrodStrings.end(); i++) {
                                    int iface = boost::lexical_cast<int>(boost::trim_copy(*i));
                                    activeScrods.push_back(iface);
                                }
                                std::map<int, DspCardInfo> cards;
                                for (auto i = activeScrods.begin(); i != activeScrods.end(); i++) {
                                    int card = *i / 4;
                                    int chan = *i % 4;
                                    if (cards.find(card) == cards.end()) {
                                        DspCardInfo c;
                                        bzero(&c, sizeof(DspCardInfo));
                                        cards[card] = c;
                                    }
                                    string delim = boost::lexical_cast<string>(*i);
                                    cards[card].id = card;
                                    if (state->getValue(LISTNER_PREFACE + delim + LISTNER_CONNECTED) == "1")
                                        cards[card].chan |= 1 << chan;
                                }
                                DspInfoHeader h;
                                h.cardNum = cards.size();
                                ::write(client, (char *) &h, sizeof(DspInfoHeader));
                                for (auto i = cards.begin(); i != cards.end(); i++) {
                                    DspCardInfo info = i->second;
                                    ::write(client, (char *) &info, sizeof(DspCardInfo));
                                }
                                break;
                            }

                                //all of the cajipci getters
                            case TRG_MIN_GET:
                            case CAL_DELAY_GET:
                            case TRG_MASK_GET:
                            case TRG_COUNT_GET:
                            case TRG_VETO_EN_GET:
                            case TRG_VETO_GET:
                            case CAL_EN_GET:
                            case CAL_TRG_DELAY_GET:
                            case CAL_COARSE_DELAY_GET:
                            case GET_MIN_A:
                            case GET_MAX_A:
                            case GET_MIN_B:
                            case GET_MAX_B:
                            case GET_MAX_DELAY_AB:
                            case GET_MIN_DELAY_AB:
                            case GET_MIN_C:
                            case GET_MAX_C:
                            case GET_TRG_EN:
                            case GET_PRESCALE_A:
                            case GET_PRESCALE_B:
                            case GET_PRESCALE_C:
                            case GET_PRESCALE_AB:
                            case GET_TRG_LINK_STATUS:
                            case GET_LIVE_TIME:
                            case GET_TRG_A_RATE:
                            case GET_TRG_B_RATE:
                            case GET_TRG_C_RATE:
                            case GET_TRG_AB_RATE: {
                                Trigger *t = Trigger::Instance();
                                DspResponce resp;
                                resp.address = 0;
                                resp.ok = true;
                                if (c.op == TRG_MIN_GET) resp.arg = t->getTriggerMin();
                                else if (c.op == CAL_DELAY_GET) resp.arg = t->getDelay();
                                else if (c.op == TRG_MASK_GET) resp.arg = t->getTriggerMask();
                                else if (c.op == TRG_COUNT_GET) resp.arg = t->getTriggerCount();
                                else if (c.op == TRG_VETO_EN_GET) resp.arg = t->getVetoEnabled();
                                else if (c.op == TRG_VETO_GET) resp.arg = t->getVeto();
                                else if (c.op == CAL_EN_GET) resp.arg = t->getCal();
                                else if (c.op == CAL_TRG_DELAY_GET) resp.arg = t->getTriggerDelay();
                                else if (c.op == CAL_COARSE_DELAY_GET) resp.arg = t->getCoarseDelay();
                                else if (c.op == GET_MIN_A) resp.arg = t->getMinA();
                                else if (c.op == GET_MAX_A) resp.arg = t->getMaxA();
                                else if (c.op == GET_MIN_B) resp.arg = t->getMinB();
                                else if (c.op == GET_MAX_B) resp.arg = t->getMaxB();
                                else if (c.op == GET_MIN_DELAY_AB) resp.arg = t->getMinDelayAB();
                                else if (c.op == GET_MAX_DELAY_AB) resp.arg = t->getMaxDelayAB();
                                else if (c.op == GET_MIN_C) resp.arg = t->getMinC();
                                else if (c.op == GET_MAX_C) resp.arg = t->getMaxC();
                                else if (c.op == GET_TRG_EN) resp.arg = t->getEnable();
                                else if (c.op == GET_PRESCALE_A) resp.arg = t->getPrescaleA();
                                else if (c.op == GET_PRESCALE_B) resp.arg = t->getPrescaleB();
                                else if (c.op == GET_PRESCALE_C) resp.arg = t->getPrescaleC();
                                else if (c.op == GET_PRESCALE_AB) resp.arg = t->getPrescaleAB();
                                else if (c.op == GET_TRG_LINK_STATUS) resp.arg = t->getTrgLinkStatus();
                                else if (c.op == GET_LIVE_TIME) resp.arg = t->getLiveTime();
                                else if (c.op == GET_TRG_A_RATE) resp.arg = t->getTrgARate();
                                else if (c.op == GET_TRG_B_RATE) resp.arg = t->getTrgBRate();
                                else if (c.op == GET_TRG_C_RATE) resp.arg = t->getTrgCRate();
                                else if (c.op == GET_TRG_AB_RATE) resp.arg = t->getTrgABRate();
                                resp.card = 0;
                                resp.chan = 0;
                                write(client, (char *) &resp, sizeof(DspResponce));
                                break;
                            }
                            case TRG_MIN_SET:
                            case CAL_DELAY_SET:
                            case TRG_MASK_SET:
                            case TRG_VETO_EN_SET:
                            case TRG_VETO_CLEAR:
                            case CAL_EN_SET:
                            case CAL_TRG_DELAY_SET:
                            case CAL_COARSE_DELAY_SET:
                            case TRG_SOFT:
                            case SET_MIN_A:
                            case SET_MAX_A:
                            case SET_MIN_B:
                            case SET_MAX_B:
                            case SET_MAX_DELAY_AB:
                            case SET_MIN_DELAY_AB:
                            case SET_MIN_C:
                            case SET_MAX_C:
                            case SET_TRG_EN:
                            case SET_PRESCALE_A:
                            case SET_PRESCALE_B:
                            case SET_PRESCALE_C:
                            case SET_PRESCALE_AB: {
                                Trigger *t = Trigger::Instance();
                                DspResponce resp;
                                resp.address = 0;
                                resp.ok = true;
                                if (c.op == TRG_MIN_SET) {
                                    resp.arg = t->getTriggerMin();
                                    t->setTriggerMin(c.arg);
                                }
                                else if (c.op == CAL_DELAY_SET) {
                                    resp.arg = t->getDelay();
                                    t->setDelay(c.arg);
                                }
                                else if (c.op == TRG_MASK_SET) {
                                    resp.arg = t->getTriggerMask();
                                    t->setTriggerMask(c.arg);
                                }
                                else if (c.op == TRG_VETO_EN_SET) {
                                    resp.arg = t->getVetoEnabled();
                                    t->enableVeto(c.arg);
                                }
                                else if (c.op == TRG_VETO_CLEAR) { t->clearVeto(); }
                                else if (c.op == CAL_EN_SET) {
                                    resp.arg = t->getCal();
                                    t->enableCal(c.arg);
                                }
                                else if (c.op == CAL_TRG_DELAY_SET) {
                                    resp.arg = t->getTriggerDelay();
                                    t->setTriggerDelay(c.arg);
                                }
                                else if (c.op == CAL_COARSE_DELAY_SET) {
                                    resp.arg = t->getCoarseDelay();
                                    t->setCoarseDelay(c.arg);
                                }
                                else if (c.op == TRG_SOFT) {
                                    resp.arg = 1;
                                    t->softTrigger();
                                }
                                else if (c.op == SET_MIN_A) {
                                    resp.arg = t->getMinA();
                                    t->setMinA(c.arg);
                                }
                                else if (c.op == SET_MAX_A) {
                                    resp.arg = t->getMaxA();
                                    t->setMaxA(c.arg);
                                }
                                else if (c.op == SET_MIN_B) {
                                    resp.arg = t->getMinB();
                                    t->setMinB(c.arg);
                                }
                                else if (c.op == SET_MAX_B) {
                                    resp.arg = t->getMaxB();
                                    t->setMaxB(c.arg);
                                }
                                else if (c.op == SET_MIN_DELAY_AB) {
                                    resp.arg = t->getMinDelayAB();
                                    t->setMinDelayAB(c.arg);
                                }
                                else if (c.op == SET_MAX_DELAY_AB) {
                                    resp.arg = t->getMaxDelayAB();
                                    t->setMaxDelayAB(c.arg);
                                }
                                else if (c.op == SET_MIN_C) {
                                    resp.arg = t->getMinC();
                                    t->setMinC(c.arg);
                                }
                                else if (c.op == SET_MAX_C) {
                                    resp.arg = t->getMaxC();
                                    t->setMaxC(c.arg);
                                }
                                else if (c.op == SET_TRG_EN) {
                                    resp.arg = t->getEnable();
                                    t->setEnable(c.arg);
                                }
                                else if (c.op == SET_PRESCALE_A) {
                                    resp.arg = t->getPrescaleA();
                                    t->setPrescaleA(c.arg);
                                }
                                else if (c.op == SET_PRESCALE_B) {
                                    resp.arg = t->getPrescaleB();
                                    t->setPrescaleB(c.arg);
                                }
                                else if (c.op == SET_PRESCALE_C) {
                                    resp.arg = t->getPrescaleC();
                                    t->setPrescaleC(c.arg);
                                }
                                else if (c.op == SET_PRESCALE_AB) {
                                    resp.arg = t->getPrescaleAB();
                                    t->setPrescaleAB(c.arg);
                                }
                                resp.card = 0;
                                resp.chan = 0;
                                write(client, (char *) &resp, sizeof(DspResponce));
                                break;
                            }
                            case STORAGE_UPGRADE: {
                                int numSubscriptions = c.arg;
                                DspSubscription info;
                                std::map<uint16_t, boost::array<uint32_t, 4> > tokens;
                                while (numSubscriptions != 0) {
                                    if (::read(client, (char *) &info, sizeof(info)) < 0)
                                        throw std::runtime_error("Register : Bad  upgrade command from clien");
                                    for (int col = 0; col < 4; col++) {
                                        tokens[info.scrod][0] = info.row0;
                                        tokens[info.scrod][1] = info.row1;
                                        tokens[info.scrod][2] = info.row2;
                                        tokens[info.scrod][3] = info.row3;
                                    }
                                    numSubscriptions--;
                                }
                                it->upgraded = true;
                                uint port = boost::lexical_cast<uint>(state->getValue(SETTING_PORT_CLIENT));
                                it->addr.sin_port = htons(port);
                                it->udpFd = ::socket(AF_INET, SOCK_DGRAM, 0);
                                connect(it->udpFd, (struct sockaddr *) &(it->addr), sizeof(it->addr));
                                s->addClent(it->udpFd, tokens);
                                break;
                            }
                            case STORAGE_ROTATE: {
                                s->rotateStorageFile();
                                DspResponce resp;
                                resp.address = 0;
                                resp.ok = true;
                                resp.card = 0;
                                resp.chan = 0;
                                resp.arg = 0;
                                ::write(client, (char *) &resp, sizeof(DspResponce));
                                break;
                            }
                            case ADVANCE_MOTOR: {
                                int position_to_advance = c.card;
                                m.moveUp(position_to_advance);
                                DspResponce resp;
                                resp.address = 0;
                                resp.ok = true;
                                resp.card = s->getLastEventNumber();;
                                resp.chan = 0;
                                resp.arg = 0;
                                ::write(client, (char *) &resp, sizeof(DspResponce));
                            }
                            default:
                                throw std::runtime_error("Register : Bad command from clien");
                        }

                    }
                    catch (std::runtime_error &e) {
                        DspResponce rsp;
                        rsp.address = c.address;
                        rsp.ok = false;
                        rsp.arg = BAD_COMMAND_ERROR;
                        rsp.card = c.card;
                        rsp.chan = c.chan;
                        ::write(client, &rsp, sizeof(DspResponce));
                        //cout << e.what() << endl;

                    }
                }
                it++;
            }


        }
    }
    catch (boost::thread_interrupted &e) {
        //We have been interupted from the main thread;
    }
    cout << "Register : Done" << endl;

}


RegisterInterface::ScrodRegisterInterface *RegisterInterface::findInterface(int id) {
    for (int i = 0; i < scrods.size(); i++) {
        if (scrods[i].id == id)
            return &scrods[i];
    }
    throw std::runtime_error("Register : Could not locate interface id");
}



