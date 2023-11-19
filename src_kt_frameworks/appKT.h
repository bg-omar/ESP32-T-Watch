//
// Created by mr on 11/18/2023.
//

#ifndef ARDUINO_KT_WATCH_APPKT_H
#define ARDUINO_KT_WATCH_APPKT_H

#include "config.h"

typedef struct {
    lv_obj_t *hour;
    lv_obj_t *minute;
    lv_obj_t *second;
} str_datetime_t;

static str_datetime_t g_data;
class appKT {
private:
    static PCF8563_Class *rtcKT;
public:
    static void KT();
};


#endif //ARDUINO_KT_WATCH_APPKT_H
