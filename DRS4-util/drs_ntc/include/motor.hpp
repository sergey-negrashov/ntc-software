//
// Created by tusk on 11/25/16.
//

#ifndef DRS_NTC_MOTOR_HPP
#define DRS_NTC_MOTOR_HPP


class Motor {
public:
    Motor();
    void moveUp(int steps);
private:
    int fd;
};


#endif //DRS_NTC_MOTOR_HPP
