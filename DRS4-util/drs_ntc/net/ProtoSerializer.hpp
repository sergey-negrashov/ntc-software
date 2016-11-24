//
// Created by tusk on 11/19/16.
//

#ifndef DRS_NTC_PROTOSERIALIZER_HPP
#define DRS_NTC_PROTOSERIALIZER_HPP

#include "../proto/ntc.pb.h"
#include "DRS.h"
#include <vector>
#include <DRS.h>

class ProtoMotorPosition {
public:
    ProtoMotorPosition(int position);
    void addEvent(DRSBoard* b, int id);
    std::string toString();
private:
    int _position;
    ntc::net::MotorPosition _store;
};


#endif //DRS_NTC_PROTOSERIALIZER_HPP
