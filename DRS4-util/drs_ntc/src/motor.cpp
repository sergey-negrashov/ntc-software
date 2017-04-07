//
// Created by tusk on 11/25/16.
//
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include "motor.hpp"

using namespace std;

Motor::Motor() {
    fd = open( "/dev/ttyUSB0", O_RDWR);
    if(fd < 0){
        cout << "Could not open /dev/ttyUSB0" << endl;
        exit(1);
    }
}

void Motor::moveUp(int steps) {
    string cmd = "C E I1M" + to_string(steps) + ",R";
    ::write(fd, cmd.c_str(), cmd.length());
    char c = 'y';
    while (c != '^'){
        ::read(fd,&c, sizeof(char));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
