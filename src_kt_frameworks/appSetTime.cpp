//
// Created by mr on 11/16/2023.
//

#include "appSetTime.h"
#include "config.h"


uint8_t appSetTime::hh, appSetTime::mm = 0;


// prtTime will display the current selected time and highlight
// the current digit to be updated in yellow
void appSetTime::prtTime(byte digit) {
    TTGOClass *getWatch = TTGOClass::getWatch();
    RTC_Date tnow = getWatch->rtc->getDateTime();

    uint8_t pthh = tnow.hour;
    uint8_t ptmm = tnow.minute;

    getWatch->tft->fillRect(0, 0, 100, 34, TFT_BLACK);
    if (digit == 1)   getWatch->tft->setTextColor(TFT_YELLOW);
    else   getWatch->tft->setTextColor(TFT_WHITE);
    getWatch->tft->drawNumber(int(pthh / 10), 5, 5, 2);
    if (digit == 2)   getWatch->tft->setTextColor(TFT_YELLOW);
    else   getWatch->tft->setTextColor(TFT_WHITE);
    getWatch->tft->drawNumber(pthh % 10, 25, 5, 2);
    getWatch->tft->setTextColor(TFT_WHITE);
    getWatch->tft->drawString(":", 45, 5, 2);
    if (digit == 3)   getWatch->tft->setTextColor(TFT_YELLOW);
    else   getWatch->tft->setTextColor(TFT_WHITE);
    getWatch->tft->drawNumber(int(ptmm / 10), 65 , 5, 2);
    if (digit == 4)   getWatch->tft->setTextColor(TFT_YELLOW);
    else   getWatch->tft->setTextColor(TFT_WHITE);
    getWatch->tft->drawNumber(ptmm % 10, 85, 5, 2);
}

// getTnum takes care of translating where we pressed into
// a number that was pressed. Returns -1 for no press
// and 13 for DONE
int appSetTime::getTnum() {
    TTGOClass *getWatch = TTGOClass::getWatch();
    int16_t x, y;
    if (!getWatch->getTouch(x, y)) return - 1;
    if (y < 85) {
        if (x < 80) return 1;
        else if (x > 160) return 3;
        else return 2;
    }
    else if (y < 135) {
        if (x < 80) return 4;
        else if (x > 160) return 6;
        else return 5;
    }
    else if (y < 185) {
        if (x < 80) return 7;
        else if (x > 160) return 9;
        else return 8;
    }
    else if (x < 80) return 0;
    else return 13;
}



void appSetTime::setTime() {

    TTGOClass *getWatch = TTGOClass::getWatch();
    // Get the current info
    RTC_Date tnow = getWatch->rtc->getDateTime();

    uint8_t sthh = tnow.hour;
    uint8_t stmm = tnow.minute;
    uint8_t stdday = tnow.day;
    uint8_t stmmonth = tnow.month;
    uint16_t styyear = tnow.year;
    custom_log(" +++++++++++++++++++++++ setTime::appSetTime() tnow: %d : %d\n", tnow.hour, tnow.minute);
//Set up the interface buttons

    getWatch->tft->fillScreen(TFT_BLACK);
    getWatch->tft->fillRect(0, 35, 80, 50, TFT_BLUE);
    getWatch->tft->fillRect(161, 35, 78, 50, TFT_BLUE);
    getWatch->tft->fillRect(81, 85, 80, 50, TFT_BLUE);
    getWatch->tft->fillRect(0, 135, 80, 50, TFT_BLUE);
    getWatch->tft->fillRect(161, 135, 78, 50, TFT_BLUE);
    getWatch->tft->fillRect(0, 185, 80, 50, TFT_BLUE);
    getWatch->tft->setTextColor(TFT_GREEN);
    getWatch->tft->drawNumber(1, 30, 40, 2);
    getWatch->tft->drawNumber(2, 110, 40, 2);
    getWatch->tft->drawNumber(3, 190, 40, 2);
    getWatch->tft->drawNumber(4, 30, 90, 2);
    getWatch->tft->drawNumber(5, 110, 90, 2);
    getWatch->tft->drawNumber(6, 190, 90, 2);
    getWatch->tft->drawNumber(7, 30, 140, 2);
    getWatch->tft->drawNumber(8, 110, 140, 2);
    getWatch->tft->drawNumber(9, 190, 140, 2);
    getWatch->tft->drawNumber(0, 30, 190, 2);
    getWatch->tft->fillRoundRect(120, 200, 119, 39, 6, TFT_WHITE);
    getWatch->tft->setTextSize(2);
    getWatch->tft->setCursor(0, 0);

    getWatch->tft->setCursor(155, 210);
    getWatch->tft->setTextColor(TFT_BLACK);
    getWatch->tft->print("DONE");
    getWatch->tft->setTextColor(TFT_WHITE);
    int wl = 0; // Track the current number selected
    byte curnum = 1;  // Track which digit we are on

    prtTime(curnum); // Display the time for the current digit

    while (wl != 13) {
        wl = getTnum();
        if (wl != -1) {
            custom_log(" ---------------> digit location: %d \n", curnum);
            custom_log(" ---> getTnum(): %d \n", wl);
            switch (curnum) {
                case 1:
                    appSetTime::hh = wl * 10 + sthh % 10;
                    custom_log(" ---> hh: %d \n", hh);
                    break;
                case 2:
                    appSetTime::hh = int(hh / 10) * 10 + wl;
                    custom_log(" ---> hh: %d \n", hh);
                    break;
                case 3:
                    appSetTime::mm = wl * 10 + stmm % 10;
                    custom_log(" ---> mm: %d \n", mm);
                    break;
                case 4:
                    appSetTime::mm = int(mm / 10) * 10 + wl;
                    custom_log(" ---> mm: %d \n", mm);
                    break;
            }
            while (getTnum() != -1) {}
            curnum += 1;
            prtTime(curnum);
            if (curnum > 4) wl = 13;
        }
    }
    while (getTnum() != -1) {}
    getWatch->rtc->setDateTime(styyear, stmmonth, stdday, hh, mm, 0);
    getWatch->tft->fillScreen(TFT_BLACK);
}





