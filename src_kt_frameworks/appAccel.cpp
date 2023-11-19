//
// Created by mr on 11/18/2023.
//

#include "config.h"
#include "appAccel.h"

void appAccel::accel() {
    TTGOClass *watch = TTGOClass::getWatch();
    watch->bma->begin();
    watch->bma->enableAccel();
    watch->tft->fillScreen(TFT_BLACK);
    int16_t x, y;
    int16_t xpos, ypos;

    Accel acc;

    while (!watch->getTouch(x, y)) { // Wait for touch to exit

        watch->bma->getAccel(acc);
        xpos = acc.x;
        ypos = acc.y;
        watch->tft->fillCircle(xpos / 10 + 119, ypos / 10 + 119, 10, TFT_RED); // draw dot
        delay(100);
        watch->tft->fillCircle(xpos / 10 + 119, ypos / 10 + 119, 10, TFT_BLACK); // erase previous dot
    }

    while (watch->getTouch(x, y)) {}  // Wait for release to return to the clock

    watch->tft->fillScreen(TFT_BLACK); // Clear screen
}