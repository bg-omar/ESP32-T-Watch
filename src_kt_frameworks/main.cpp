// An Arduino based framework for the Lilygo T-Watch 2020
// Much of the code is based on the sample apps for the
// T-watch that were written and copyrighted by Lewis He.
//(Copyright (c) 2019 lewis he)

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include "config.h"
#include <SPI.h>
#include <soc/rtc.h>
#include <Wire.h>
#include <FS.h>
#include <ctime>
#include <WiFi.h>
#include <SPI.h>
#include <soc/rtc.h>


#include "appAccel.h"
#include "appBattery.h"
#include "appJsat.h"
#include "appKT.h"
#include "appSetTime.h"
#include "appTouch.h"
#include "appMario.h"
#include "appDisplayTime.h"


#include "gui.h"
#include "secret.h"


TTGOClass *watch = TTGOClass::getWatch();
PCF8563_Class *rtc;
AXP20X_Class *power = watch->power;



bool marioLooper, ktLooper = false;




static uint8_t ss ;


uint32_t targetTime = 0;       // for next 1-second display update
uint32_t clockUpTime = 0;      // track the time the clock is displayed
const int maxApp = 8; // number of apps
String appName[maxApp] = {"Clock", "Battery", "Jupiter", "Accel", "SetTime","Touch", "Mario", "KT"}; // app names

void setMenuDisplay(int mSel) {
    int curSel = 0;
    // Display mode header
    watch->tft->fillScreen(TFT_MAGENTA);
    watch->tft->fillRect(0, 80, 239, 80, TFT_PURPLE);

    // Display apps
    if (mSel == 0) curSel = maxApp - 1;
    else curSel = mSel - 1;

    watch->tft->setTextSize(2);
    watch->tft->setTextColor(TFT_GREEN);
    watch->tft->setCursor(50, 30);
    watch->tft->println(appName[curSel]);

    watch->tft->setTextSize(3);
    watch->tft->setTextColor(TFT_RED);
    watch->tft->setCursor(40, 110);
    watch->tft->println(appName[mSel]);

    if (mSel == maxApp - 1) curSel = 0;
    else curSel = mSel + 1;

    watch->tft->setTextSize(2);
    watch->tft->setTextColor(TFT_GREEN);
    watch->tft->setCursor(50, 190);
    watch->tft->print(appName[curSel]);
}

uint8_t modeMenu() {
    int mSelect = 0; // The currently highlighted app
    int16_t x, y, tx, ty;

    boolean exitMenu = false; // used to stay in the menu until user selects app

    setMenuDisplay(0); // display the list of Apps

    while (!exitMenu) {
        if (watch->getTouch(x, y)) { // If you have touched something...

            while (watch->getTouch(tx, ty)) {} // wait until you stop touching

            if (y >= 160) { // you want the menu list shifted up
                mSelect += 1;
                if (mSelect == maxApp) mSelect = 0;
                setMenuDisplay(mSelect);
            }

            if (y <= 80) { // you want the menu list shifted down
                mSelect -= 1;
                if (mSelect < 0) mSelect = maxApp - 1;
                setMenuDisplay(mSelect);
            }
            if (y > 80 && y < 160) { // You selected the middle
                exitMenu = true;
            }
        }
    }
    //Return with mSelect containing the desired mode
    watch->tft->fillScreen(TFT_BLACK);
    return mSelect;
}



void setup()
{
    Serial.begin(115200);
    Serial.println("Woked-up!");
    watch = TTGOClass::getWatch();
    appMario::setupMario();
    // Set 20MHz operating speed to reduce power consumption
    //setCpuFrequencyMhz(20);


}


void loop()
{
    if (marioLooper) {
        int16_t x, y;
        while (!watch->getTouch(x, y)) { appMario::marioLoop(); }// Wait for touch
        while (watch->getTouch(x, y)) {}
        marioLooper = false;
        custom_log(" ---> marioLooper: %4d \n", marioLooper); // Wait for release to exit
    }

    if (ktLooper) {
        int16_t x, y;
        while (!watch->getTouch(x, y)) { lv_task_handler(); }// Wait for touch}
        while (watch->getTouch(x, y)) {}
        ktLooper = false;
        custom_log(" ---> ktLooper: %4d \n", ktLooper); // Wait for release to exit
    }

    if (targetTime < millis()) {
        targetTime = millis() + 1000;
        appDisplayTime::displayTime(ss == 0); // Call every second but only update time every minute
    }

    int16_t x, y;
    if (watch->getTouch(x, y)) {
        while (watch->getTouch(x, y)) {} // wait for user to release
        marioLooper = false;
        ktLooper = false;

        switch (modeMenu()) { // Call modeMenu. The return is the desired app number
            case 0: // Zero is the clock, just exit the switch
                appDisplayTime::displayTime(true);
                break;
            case 1:
                appBattery::battery();
                break;
            case 2:
                appJsat::jSats();
                break;
            case 3:
                appAccel::accel();
                break;
            case 4:
                appSetTime::setTime();
                break;
            case 5:
                appTouch::touch();
                break;
            case 6:
                marioLooper = true;
                appMario::setupGUI();
                custom_log(" ---> marioLooper: %4d\n", marioLooper);
                appMario::marioLoop();
                break;
            case 7:
                ktLooper = true;
                custom_log(" ---> ktLooper: %4d\n", ktLooper);
                watch->tft->fillScreen(TFT_BLACK);
                appKT::KT();
                break;

        }

    }
}


