#ifndef __CONFIG_H
#define __CONFIG_H

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include "lvgl.h"

#define FPS 30


enum ModTimeUnit
{
    set = 0,
    hour,
    minute
};
enum Gestures
{
    none = 0,
    left,
    right,
    up,
    down
};

#ifndef SIMULATOR

#define custom_log Serial.printf

#define LILYGO_WATCH_2020_V3 // To use T-Watch2020 , please uncomment this line
#define LILYGO_WATCH_HAS_BUTTON
#define LILYGO_WATCH_LVGL    //To use LVGL, you need to enable the macro LVGL

#undef LILYGO_WATCH_LVGL
#define HARDWARE_NAME   "T-Watch2020V3"
#define RES_X_MAX       240
#define RES_Y_MAX       240
#define USE_PSRAM_ALLOC_LVGL                    /** @brief enabled LVGL to use PSRAM */

#define WATCH_FLAG_SLEEP_MODE _BV(1)
#define WATCH_FLAG_SLEEP_MODE _BV(1)
#define WATCH_FLAG_SLEEP_EXIT _BV(2)
#define WATCH_FLAG_BMA_IRQ _BV(3)
#define WATCH_FLAG_AXP_IRQ _BV(4)

/**
 * Enable non-latin languages support
 */

#define USE_EXTENDED_CHARSET CHARSET_CYRILLIC

/**
 * firmeware version string
 */

#define __FIRMWARE__            "2023091101"

/**
 * Allows to include config.h from C code
 */
#ifdef __cplusplus
    #ifdef M5PAPER
    #elif defined( LILYGO_WATCH_2020_V1 ) || defined( LILYGO_WATCH_2020_V2 ) || defined( LILYGO_WATCH_2020_V3 )
    #include <LilyGoWatch.h>
    #endif
    #define _CONFIG_H
    #endif
#endif
#endif //__CONFIG_H
