//
// Created by mr on 11/18/2023.
//
#include "config.h"
#include "appTouch.h"

void appTouch::touch() {
    TTGOClass *watch = TTGOClass::getWatch();
    uint32_t endTime = millis() + 10000; // Timeout at 10 seconds
    int16_t x, y;
    watch->tft->fillScreen(TFT_BLACK);

    while (endTime > millis()) {
        watch->getTouch(x, y);
        watch->tft->fillRect(98, 100, 70, 85, TFT_BLACK);
        watch->tft->setCursor(80, 100);
        watch->tft->print("X:");
        watch->tft->println(x);
        watch->tft->setCursor(80, 130);
        watch->tft->print("Y:");
        watch->tft->println(y);
        delay(25);
    }

    while (watch->getTouch(x, y)) {}  // Wait for release to exit
    watch->tft->fillScreen(TFT_BLACK);
}
