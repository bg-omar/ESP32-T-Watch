//
// Created by mr on 11/16/2023.
//

#ifndef ARDUINO_KT_WATCH_APPSETTIME_H
#define ARDUINO_KT_WATCH_APPSETTIME_H
#include <cstdint>
#include "config.h"
#include <soc/rtc.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>



class appSetTime {
private:
    static uint8_t hh, mm;
    PCF8563_Class *rtc;
public:
    static void setTime();
    static void prtTime(byte digit);
    static int getTnum();
};


#endif //ARDUINO_KT_WATCH_SETTIME_H
