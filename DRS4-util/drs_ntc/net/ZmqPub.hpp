//
// Created by tusk on 11/19/16.
//

#ifndef DRS_NTC_ZMQPUB_HPP
#define DRS_NTC_ZMQPUB_HPP
#include <string>
#include "zmq.hpp"
#include "ProtoSerializer.hpp"

class ZmqPub {
public:
    ZmqPub(std::string server);
    ~ZmqPub();
    void sendData(ProtoMotorPosition &a);
private:
    zmq::context_t *_context;
    zmq::socket_t *_publisher;
};


#endif //DRS_NTC_ZMQPUB_HPP
