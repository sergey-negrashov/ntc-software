//
// Created by tusk on 11/19/16.
//

#include "ZmqPub.hpp"
#include "zhelpers.hpp"
ZmqPub::ZmqPub(std::string server){
    _context = new zmq::context_t(1);
    _publisher = new zmq::socket_t(*_context, ZMQ_PUB);
    _publisher->connect(server);
}

ZmqPub::~ZmqPub() {
    delete _publisher;
    delete _context;
}

void ZmqPub::sendData(ProtoMotorPosition &a) {
    s_sendmore(*_publisher,"0");
    s_send(*_publisher,a.toString());
}

