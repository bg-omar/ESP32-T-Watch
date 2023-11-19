//
// Created by mr on 11/19/2023.
//

#include "appDisplayTime.h"

TTGOClass *watchTime = TTGOClass::getWatch();

void appDisplayTime::displayTime(boolean fullUpdate) {
    byte xcolon = 0; // location of the colon
    //    watch->begin();
    watchTime->tft->setTextFont(1);
    watchTime->tft->setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour
    byte xpos = 40; // Stating position for the display
    byte ypos = 90;

    // Get the current data
    RTC_Date tnow = watchTime->rtc->getDateTime();

    uint8_t dthh = tnow.hour;
    uint8_t  dtmm = tnow.minute;
    uint8_t  dtss = tnow.second;
    uint8_t dtdday = tnow.day;
    uint8_t dtmmonth = tnow.month;
    uint16_t dtyyear = tnow.year;

    watchTime->tft->setTextSize(1);

    if (fullUpdate) {
        // Font 7 is a 7-seg display but only contains
        // characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .

        watchTime->tft->setTextColor(0x39C4); // Set desired color
        watchTime->tft->drawString("88:88", xpos, ypos, 7);
        watchTime->tft->setTextColor(0xFBE0); // Orange

        if (dthh < 10) xpos += watchTime->tft->drawChar('0', xpos, ypos, 7);
        xpos += watchTime->tft->drawNumber(dthh, xpos, ypos, 7);
        xcolon = xpos + 3;
        xpos += watchTime->tft->drawChar(':', xcolon, ypos, 7);
        if (dtmm < 10) xpos += watchTime->tft->drawChar('0', xpos, ypos, 7);
        watchTime->tft->drawNumber(dtmm, xpos, ypos, 7);
    }

    if (dtss % 2) { // Toggle the colon every second
        watchTime->tft->setTextColor(0x39C4);
        xpos += watchTime->tft->drawChar(':', xcolon, ypos, 7);
        watchTime->tft->setTextColor(0xFBE0);
    } else {
        watchTime->tft->drawChar(':', xcolon, ypos, 7);
    }
    watchTime->tft->setTextSize(3);
    watchTime->tft->setCursor( 10, 210);

    watchTime->tft->print(dtmmonth);
    watchTime->tft->print("/");
    watchTime->tft->print(dtdday);
    watchTime->tft->print("/");
    watchTime->tft->print(dtyyear);
}