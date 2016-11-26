//
// Created by tusk on 11/19/16.
//

#include "ProtoSerializer.hpp"
#include <iostream>

using namespace std;

ProtoMotorPosition::ProtoMotorPosition(int position){
    _position = position;
    _store = ntc::net::MotorPosition();
    _store.set_motorpos(position);
}

void ProtoMotorPosition::addEvent(DRSBoard *b, int id) {
    float time_array[1024];
    float wave_array[1024];
    ntc::net::Event *e= _store.add_events();
    e->set_eventid(id);
    for(int i = 0; i < 4; i++) {
        ntc::net::Channel *c = e->add_channel();
        c->set_channel(i);
        b->GetTime(0, i*2, b->GetTriggerCell(0), time_array);
        b->GetWave(0, i*2, wave_array);
        for(int j = 0; j < 1024; j++) {
            c->add_time(time_array[j]);
            c->add_voltage(wave_array[j]);
        }
    }
}

std::string ProtoMotorPosition::toString() {
    std::string out;
    _store.SerializeToString(&out);
    return out;
}