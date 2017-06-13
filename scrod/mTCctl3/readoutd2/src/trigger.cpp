/*
 * This file is part of readoutd.
 *
 * readoutd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * readoutd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with readoutd.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright Sergey Negrashov 2014.
*/

#include "../lib/trigger.hpp"

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

Trigger* Trigger::special = NULL;
#define CAJIPCIDEV "/dev/ttyCAJIPCI"
#define SET_REG_CMD 0x80
#define GET_REG_CMD 0

using namespace std;

inline int set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    //memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf ("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag |= CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    //tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        printf ("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

inline void set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf ("error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        printf ("error %d setting term attributes", errno);
}

inline uint8_t getReg(int fd, uint8_t addr, bool &ok)
{
    uint8_t toSend = addr | GET_REG_CMD;
    uint8_t rd_byte;

    int retry = 0;
    ok = true;
    while(retry < 10)
    {
        while(read (fd, &rd_byte, 1) > 0){}
        write (fd, &toSend, 1);
        usleep(1000);
        int n = read (fd, &rd_byte, 1);
        usleep(1000);
        retry++;
        if(n == 1)
            return rd_byte;
    }
    ok = false;
    return -1;
}

inline void setReg(int fd, uint8_t addr, uint8_t value)
{
    uint8_t rd_byte;
    while(read (fd, &rd_byte, 1) > 0){}
    uint8_t toSend = addr | SET_REG_CMD;
    write (fd, &toSend, 1);
    usleep(1000);
    toSend = value;
    write (fd, &toSend, 1);
    usleep(1000);
}

Trigger* Trigger::Instance()
{
    if(!special)
        special = new Trigger();
    return special;
}

Trigger::Trigger()
{
    ok = true;
    fd_ = open (CAJIPCIDEV, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY);
    if(fd_ < 0 )
    {
        ok = false;
        return;
    }
    set_interface_attribs (fd_, B115200, 0);    // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (fd_, 0);               	 // set non blocking
    getReg(fd_, 0, ok);

}

void Trigger::setTriggerMask(uint32_t mask)
{
    uint32_t low;
    uint32_t high;
    low = mask & 0xFF;
    high = (mask >> 8) & 0xf;
    lock_.lock();
    //get min scrods;
    uint8_t currentHigh = getReg(fd_, MASK_HIGH_MIN_ADDR, ok);
    currentHigh &= 0xf0;
    currentHigh |= high;
    setReg(fd_, MASK_HIGH_MIN_ADDR, currentHigh);
    setReg(fd_, MASK_LOW_ADDR, low);
    lock_.unlock();
}

void Trigger::setTriggerMin(uint32_t min)
{
    min &= 0xF;
    min = min << 4;
    lock_.lock();
    uint8_t currentHigh = getReg(fd_, MASK_HIGH_MIN_ADDR, ok);
    currentHigh &= 0x0F;
    currentHigh |= min;
    setReg(fd_, MASK_HIGH_MIN_ADDR, currentHigh);
    lock_.unlock();
}

void Trigger::softTrigger()
{
    lock_.lock();
    uint8_t currentVeto = getReg(fd_, VETO_ADDR, ok);
    currentVeto &= ~(0x1);
    setReg(fd_, VETO_ADDR, currentVeto);
    currentVeto |= 0x1;
    setReg(fd_, VETO_ADDR, currentVeto);
    lock_.unlock();
}


void Trigger::clearVeto()
{

    lock_.lock();
    uint8_t currentVeto = getReg(fd_, VETO_ADDR, ok);
    currentVeto |= (0x4);
    setReg(fd_, VETO_ADDR, currentVeto);
    lock_.unlock();
}


uint Trigger::getTriggerCount()
{
    lock_.lock();
    uint8_t currentCount = getReg(fd_, COUNTER_ADDR, ok);
    lock_.unlock();
    return currentCount;
}

uint16_t Trigger::getTriggerMask()
{
    uint16_t ret;
    lock_.lock();
    uint32_t low = getReg(fd_, MASK_LOW_ADDR, ok);
    uint32_t high = getReg(fd_, MASK_HIGH_MIN_ADDR, ok);
    lock_.unlock();
    ret = low | ((high & 0xF) << 8);
    return ret;
}

uint8_t Trigger::getTriggerMin()
{
    uint8_t ret;
    lock_.lock();
    ret = getReg(fd_, MASK_HIGH_MIN_ADDR, ok);
    lock_.unlock();
    return ret >> 4;
}

void Trigger::enableVeto(bool on)
{
    lock_.lock();
    uint8_t currentVeto = getReg(fd_, VETO_ADDR, ok);
    if(!on)
        currentVeto &= ~(0x2);
    else
        currentVeto |= 0x2;
    setReg(fd_, VETO_ADDR, currentVeto);
    lock_.unlock();
}

bool Trigger::getVetoEnabled()
{
    lock_.lock();
    uint8_t currentVeto = getReg(fd_, VETO_ADDR, ok);
    lock_.unlock();
    return currentVeto & 0x2;
}

bool Trigger::getVeto()
{
    lock_.lock();
    uint8_t currentVeto = getReg(fd_, VETO_ADDR, ok);
    lock_.unlock();
    return currentVeto & 0x8;
}

void Trigger::setDelay(uint16_t delay)
{
    delay &= 0x7FF;    //chop off the bottom 11 bits
    uint8_t  highDelay;
    uint8_t lowDelay;
    lock_.lock();
    lowDelay = delay & 0xFF;
    highDelay = (delay >> 8);
    setReg(fd_, DELAY_LOW_ADDR, lowDelay);
    setReg(fd_, DELAY_HIGH_ADDR, highDelay);
    lock_.unlock();
    return;
}

uint16_t Trigger::getDelay()
{
    uint16_t ret = 0;
    uint8_t  highDelay;
    uint8_t lowDelay;
    lock_.lock();
    lowDelay = getReg(fd_, DELAY_LOW_ADDR, ok);
    highDelay = getReg(fd_, DELAY_HIGH_ADDR, ok);
    lock_.unlock();
    ret = lowDelay;
    ret |= (uint16_t)highDelay << 8;
    ret &= 0x7FF;
    return ret;

}

void Trigger::enableCal(bool on)
{
    lock_.lock();
    uint8_t cal;
    if(!on)
        cal = 0;
    else
        cal = 1;
    setReg(fd_, CAL_EN_REG, cal);
    lock_.unlock();
}

bool Trigger::getCal()
{
    lock_.lock();
    uint8_t cal;
    cal = getReg(fd_, CAL_EN_REG, ok);
    lock_.unlock();
    return cal & 0x1;
}

void Trigger::setTriggerDelay(uint8_t delay)
{
    lock_.lock();
    setReg(fd_, CAL_TRG_DELAY_REG, delay);
    lock_.unlock();
}

uint8_t Trigger::getTriggerDelay()
{
    lock_.lock();
    uint8_t delay  = getReg(fd_, CAL_TRG_DELAY_REG, ok);
    lock_.unlock();
    return delay;
}

void Trigger::setCoarseDelay(uint8_t delay)
{
    lock_.lock();
    setReg(fd_, CAL_DELAY_COARSE_REG, delay);
    lock_.unlock();
}

uint8_t Trigger::getCoarseDelay()
{
    lock_.lock();
    uint8_t delay  = getReg(fd_, CAL_DELAY_COARSE_REG, ok);
    lock_.unlock();
    return delay;
}

float Trigger::getTemperature()
{
    lock_.lock();
    uint16_t temp;
    temp = getReg(fd_, TEMP_LOW_REG, ok);
    temp |= (uint16_t)getReg(fd_, TEMP_HIGH_REG, ok) << 8;
    lock_.unlock();
    return temp/10.0;
}

float Trigger::getHumidity()
{
    lock_.lock();
    uint16_t hum;
    hum = getReg(fd_, HUM_LOW_REG, ok);
    hum |= (uint16_t)getReg(fd_, HUM_HIGH_REG, ok) << 8;
    lock_.unlock();
    return hum/10.0;
}

bool Trigger::getLeak()
{
    lock_.lock();
    bool ret;
    ret = getReg(fd_, LEAK_REG, ok);
    lock_.unlock();
    return ret;
}

void Trigger::set16BitReg(uint32_t val, const int addrLow, const int addrHigh) {
    uint32_t high;
    uint32_t low;
    low = val & 0xFF;
    high = (val >> 8) & 0xFF;
    lock_.lock();
    setReg(fd_, addrLow, low);
    setReg(fd_, addrHigh, high);
    lock_.unlock();
    return;
}

uint16_t Trigger::get16BitReg(const int addrLow, const int addrHigh) {
    uint16_t ret = 0;
    lock_.lock();
    uint32_t low  = getReg(fd_, addrLow, ok);
    uint32_t high = getReg(fd_, addrHigh, ok);
    lock_.unlock();
    ret = low | ((high & 0xFF) << 8);
    return ret;
}

void Trigger::set8BitReg(uint8_t val, const int addr) {
    lock_.lock();
    setReg(fd_, addr, val);
    lock_.unlock();
    return;

}

uint8_t Trigger::get8BitReg(const int addr) {
    uint8_t ret = 0;
    lock_.lock();
    ret = getReg(fd_, addr, ok);
    lock_.unlock();
    return ret;
}

