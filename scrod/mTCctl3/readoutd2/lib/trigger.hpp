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

#ifndef TRIGGER_HPP
#define TRIGGER_HPP
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include <string>

//#define OLD_CAJIPCI

class Trigger
{
public:
    static Trigger* Instance();

    void setTriggerMask(uint32_t mask);
    uint16_t getTriggerMask();

    void setTriggerMin(uint32_t min);
    uint8_t getTriggerMin();

    uint getTriggerCount();
    void softTrigger();

    void enableVeto(bool on);
    bool getVetoEnabled();
    bool getVeto();
    void clearVeto();

    void setDelay(uint16_t delay);
    uint16_t getDelay();
    void enableCal(bool on);
    bool getCal();

    void setTriggerDelay(uint8_t delay);
    uint8_t getTriggerDelay();

    void setCoarseDelay(uint8_t delay);
    uint8_t getCoarseDelay();
    bool ok;

    float getTemperature();
    float getHumidity();
    bool getLeak();

    void set16BitReg(uint32_t val, const int addrLow, const int addrHigh);
    uint16_t get16BitReg(const int addrLow, const int addrHigh);
    void set8BitReg(uint8_t val, const int addr);
    uint8_t get8BitReg(const int addr);

    void setEnable(uint8_t enable) { set8BitReg(enable, TRG_EN_REG); };
    uint8_t getEnable() { return get8BitReg(TRG_EN_REG); };

    void setMinA(uint16_t min) { set16BitReg(min, TRG_A_MIN_LOW, TRG_A_MIN_HIGH); }
    void setMaxA(uint16_t max) { set16BitReg(max, TRG_A_MAX_LOW, TRG_A_MAX_HIGH); }
    void setMinB(uint16_t min) { set16BitReg(min, TRG_B_MIN_LOW, TRG_B_MIN_HIGH); }
    void setMaxB(uint16_t max) { set16BitReg(max, TRG_B_MAX_LOW, TRG_B_MAX_HIGH); }
    void setMinC(uint16_t min) { set16BitReg(min, TRG_C_MIN_LOW, TRG_C_MIN_HIGH); }
    void setMaxC(uint16_t max) { set16BitReg(max, TRG_C_MAX_LOW, TRG_C_MAX_HIGH); }
    uint16_t getMinA() { return get16BitReg(TRG_A_MIN_LOW, TRG_A_MIN_HIGH); };
    uint16_t getMaxA() { return get16BitReg(TRG_A_MAX_LOW, TRG_A_MAX_HIGH); };
    uint16_t getMinB() { return get16BitReg(TRG_B_MIN_LOW, TRG_B_MIN_HIGH); };
    uint16_t getMaxB() { return get16BitReg(TRG_B_MAX_LOW, TRG_B_MAX_HIGH); };
    uint16_t getMinC() { return get16BitReg(TRG_C_MIN_LOW, TRG_C_MIN_HIGH); };
    uint16_t getMaxC() { return get16BitReg(TRG_C_MAX_LOW, TRG_C_MAX_HIGH); };
    void setMinDelayAB(uint16_t delay) { set16BitReg(delay, MIN_DELAY_AB_LOW, MIN_DELAY_AB_HIGH); }
    uint16_t getMinDelayAB() { return get16BitReg(MIN_DELAY_AB_LOW, MIN_DELAY_AB_HIGH); }
    void setMaxDelayAB(uint16_t delay) { set16BitReg(delay, MAX_DELAY_AB_LOW, MAX_DELAY_AB_HIGH); }
    uint16_t getMaxDelayAB() { return get16BitReg(MAX_DELAY_AB_LOW, MAX_DELAY_AB_HIGH); }

    void setPrescaleA(uint8_t val) { set8BitReg(val,TRG_A_PRESCALE); }
    uint8_t getPrescaleA() { return get8BitReg(TRG_A_PRESCALE); }
    void setPrescaleB(uint8_t val) { set8BitReg(val,TRG_B_PRESCALE); }
    uint8_t getPrescaleB() { return get8BitReg(TRG_B_PRESCALE); }
    void setPrescaleC(uint8_t val) { set8BitReg(val,TRG_C_PRESCALE); }
    uint8_t getPrescaleC() { return get8BitReg(TRG_C_PRESCALE); }
    void setPrescaleAB(uint8_t val) { set8BitReg(val,TRG_AB_PRESCALE); }
    uint8_t getPrescaleAB() { return get8BitReg(TRG_AB_PRESCALE); }
    uint16_t getTrgLinkStatus() { return get16BitReg(LINK_STATUS_LOW,LINK_STATUS_HIGH); }
    uint8_t getLiveTime() { return get8BitReg(LIVE_FRACTION); }
    uint16_t getTrgARate() { return get16BitReg(RATE_A_LOW,RATE_A_HIGH); }
    uint16_t getTrgBRate() { return get16BitReg(RATE_B_LOW,RATE_B_HIGH); }
    uint16_t getTrgCRate() { return get16BitReg(RATE_C_LOW,RATE_C_HIGH); }
    uint16_t getTrgABRate() { return get16BitReg(RATE_AB_LOW,RATE_AB_HIGH); }

private:
    Trigger();
    Trigger(Trigger const&){}
    Trigger& operator=(Trigger const&){}
    static Trigger* special;


    boost::mutex lock_;
    int fd_;
    static const int MASK_LOW_ADDR = 1;
    static const int MASK_HIGH_MIN_ADDR = 2;
    static const int VETO_ADDR = 3;
    static const int COUNTER_ADDR = 4;
    static const int DELAY_LOW_ADDR = 10;
    static const int DELAY_HIGH_ADDR = 11;
    static const int CAL_EN_REG = 12;
    static const int CAL_TRG_DELAY_REG = 13;
    static const int CAL_DELAY_COARSE_REG = 14;
    static const int TEMP_LOW_REG = 15;
    static const int TEMP_HIGH_REG = 16;
    static const int HUM_LOW_REG = 17;
    static const int HUM_HIGH_REG = 18;
    static const int LEAK_REG = 19;
    static const int TRG_EN_REG = 20;
    static const int TRG_A_MIN_LOW  = 21;
    static const int TRG_A_MIN_HIGH = 22;
    static const int TRG_A_MAX_LOW  = 23;
    static const int TRG_A_MAX_HIGH = 24;
    static const int TRG_B_MIN_LOW  = 25;
    static const int TRG_B_MIN_HIGH = 26;
    static const int TRG_B_MAX_LOW  = 27;
    static const int TRG_B_MAX_HIGH = 28;
    static const int MAX_DELAY_AB_LOW  = 29;
    static const int MAX_DELAY_AB_HIGH = 30;
    static const int TRG_C_MIN_LOW  = 31;
    static const int TRG_C_MIN_HIGH = 32;
    static const int TRG_C_MAX_LOW  = 33;
    static const int TRG_C_MAX_HIGH = 34;
    static const int TRG_A_PRESCALE = 35;
    static const int TRG_B_PRESCALE = 36;
    static const int TRG_C_PRESCALE = 37;
    static const int TRG_AB_PRESCALE = 38;
    static const int LINK_STATUS_LOW = 39;
    static const int LINK_STATUS_HIGH = 40;
    static const int LIVE_FRACTION = 41;
    static const int RATE_A_LOW  = 42;
    static const int RATE_A_HIGH = 43;
    static const int RATE_B_LOW  = 44;
    static const int RATE_B_HIGH = 45;
    static const int RATE_C_LOW  = 46;
    static const int RATE_C_HIGH = 47;
    static const int RATE_AB_LOW  = 48;
    static const int RATE_AB_HIGH = 49;
    static const int MIN_DELAY_AB_LOW  = 50;
    static const int MIN_DELAY_AB_HIGH = 51;
};

typedef boost::shared_ptr <Trigger> trg_ptr;

#endif // TRIGGER_HPP
