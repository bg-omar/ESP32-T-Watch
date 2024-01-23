//
// Created by mr on 11/18/2023.
//

#ifndef ARDUINO_KT_WATCH_APPKT_H
#define ARDUINO_KT_WATCH_APPKT_H

#include "config.h"

class appKT {
private:
    static PCF8563_Class *rtcKT;
public:
    static void KT();

    static void setupKT();
};


#endif //ARDUINO_KT_WATCH_APPKT_H
