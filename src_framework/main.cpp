// An Arduino based framework for the Lilygo T-Watch 2020
// Much of the code is based on the sample apps for the
// T-watch that were written and copyrighted by Lewis He.
//(Copyright (c) 2019 lewis he)
#include "config.h"
#include <SPI.h>
#include <soc/rtc.h>
#include <Wire.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Ticker.h>
#include <time.h>

typedef struct {
    lv_obj_t *hour;
    lv_obj_t *minute;
    lv_obj_t *second;
} str_datetime_t;


static str_datetime_t g_data;
TTGOClass *watch = nullptr;
PCF8563_Class *rtc;
LV_IMG_DECLARE(cat_png);
LV_FONT_DECLARE(cat_font);


byte xcolon = 0; // location of the colon

uint32_t targetTime = 0;       // for next 1-second display update
uint32_t clockUpTime = 0;      // track the time the clock is displayed

uint8_t hh,  mm , ss , dday ,mmonth;
uint16_t yyear; // Year is 16 bit int
void prtTime(byte) ;
int getTnum() ;

//TTGOClass *ttgo = TTGOClass::getWatch();
#define APP_TIME_ZONE   2 // I am East Coast in Daylight Savings

const int maxApp = 5; // number of apps
String appName[maxApp] = {"Clock", "Battery", "Jupiter", "Accel", "Touch"}; // app names  , "Set Time", "KT"


void jSats() {

    // Get the current info
    RTC_Date tnow = watch->rtc->getDateTime();

    uint8_t jshh = tnow.hour;
    uint8_t jsmm = tnow.minute;
    uint8_t jsdday = tnow.day;
    uint8_t jsmmonth = tnow.month;
    uint16_t jsyyear = tnow.year;

    float tDay = jsdday; // Calculate the day plus fractional day
    tDay += (jshh - APP_TIME_ZONE) / 24.0;
    tDay += jsmm / 1440.0;
    int16_t tYear = jsyyear;
    int8_t tMonth = jsmmonth;

    int16_t cYear, cMonth;
    if (tMonth < 3) {
        cYear = tYear - 1;
        cMonth = tMonth + 12;
    } else {
        cYear = tYear;
        cMonth = tMonth;
    }
    // Calculate the Julian Date offset from Epoch
    int a = cYear / 100;
    int b = 2 - a + (int)(a / 4);
    long c = 365.25 * cYear;
    long d = 30.6001 * (cMonth + 1);
    float N = b + c + d + tDay - 694025.5;

    // Calc moon positions
    float P = PI / 180;
    float MT = (358.476 + 0.9856003 * N) * P;
    float MJ = (225.328 + 0.0830853 * N) * P;
    float JJ = 221.647 + 0.9025179 * N;
    float VT = 1.92 * sin(MT) + 0.02 * sin(2 * MT);
    float VJ = 5.55 * sin(MJ) + 0.17 * sin(2 * MJ);
    float K = (JJ + VT - VJ) * P;
    float DT = sqrt(28.07 - 10.406 * cos(K));
    float Z1 = sin(K) / DT;
    float I = atan(Z1 / sqrt(1 - Z1 * Z1));
    I = I / P;
    float F = (N - DT / 173);
    float F1 = I - VJ;
    float U1 = 84.5506 + 203.405863 * F + F1;
    float U2 = 41.5015 + 101.2916323 * F + F1;
    float U3 = 109.9770 + 50.2345169 * F + F1;
    float U4 = 176.3586 + 21.4879802 * F + F1;
    float X1 = 5.906 * sin(U1 * P);
    float X2 = 9.397 * sin(U2 * P);
    float X3 = 14.989 * sin(U3 * P);
    float X4 = 26.364 * sin(U4 * P);
    // Print out results

    watch->tft->fillScreen(TFT_BLACK);
    watch->tft->setTextSize(2);
    watch->tft->setCursor( 0, 10);
    watch->tft->setTextColor(TFT_ORANGE);
    watch->tft->print(" IO: ");
    watch->tft->print(X1, 1);
    watch->tft->setCursor( 0, 30);
    watch->tft->setTextColor(TFT_BLUE);
    watch->tft->print(" EU: ");
    watch->tft->println(X2, 1);
    watch->tft->setCursor( 0, 50);
    watch->tft->setTextColor(TFT_GREEN);
    watch->tft->print(" GA: ");
    watch->tft->println(X3, 1);
    watch->tft->setCursor( 0, 70);
    watch->tft->setTextColor(TFT_YELLOW);
    watch->tft->print(" CA: ");
    watch->tft->println(X4, 1);

    //Now display them as they would appear

    watch->tft->fillCircle(119, 155, 6, TFT_RED); // Jupiter

    watch->tft->setTextColor(TFT_ORANGE);
    watch->tft->fillCircle(int(X1 * 4 + 119), 155, 2, TFT_ORANGE);
    watch->tft->drawString("I", int(X1 * 4 + 119)-3, 175 , 1);
    watch->tft->setTextColor(TFT_BLUE);
    watch->tft->fillCircle(int(X2 * 4 + 119), 155, 2, TFT_BLUE);
    watch->tft->drawString("E", int(X2 * 4 + 119)-3, 175, 1);
    watch->tft->setTextColor(TFT_GREEN);
    watch->tft->fillCircle(int(X3 * 4 + 119), 155, 2, TFT_GREEN);
    watch->tft->drawString("G", int(X3 * 4 + 119)-3, 175, 1);
    watch->tft->setTextColor(TFT_YELLOW);
    watch->tft->fillCircle(int(X4 * 4 + 119), 155, 2, TFT_YELLOW);
    watch->tft->drawString("C", int(X4 * 4 + 119)-3, 175, 1);


    // Wait for touch to return
    int16_t x, y;
    while (!watch->getTouch(x, y)) {} // Wait for touch
    while (watch->getTouch(x, y)) {}  // Wait for release
    watch->tft->fillScreen(TFT_BLACK);
}

void appAccel() {
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

void appBattery() {
    watch->tft->fillScreen(TFT_BLACK);
    watch->tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    watch->tft->drawString("BATT STATS",  35, 30, 2);
    watch->tft->setTextColor(TFT_GREEN, TFT_BLACK);

    // Turn on the battery adc to read the values
    watch->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
    // get the values
    float vbus_v = watch->power->getVbusVoltage();
    float vbus_c = watch->power->getVbusCurrent();
    float batt_v = watch->power->getBattVoltage();
    int per = watch->power->getBattPercentage();

// Print the values
    watch->tft->setCursor(0, 100);
    watch->tft->print("Vbus: "); watch->tft->print(vbus_v); watch->tft->println(" mV");
    watch->tft->setCursor(0, 130);
    watch->tft->print("Vbus: "); watch->tft->print(vbus_c); watch->tft->println(" mA");
    watch->tft->setCursor(0, 160);
    watch->tft->print("BATT: "); watch->tft->print(batt_v); watch->tft->println(" mV");
    watch->tft->setCursor(0, 190);
    watch->tft->print("Per: "); watch->tft->print(per); watch->tft->println(" %");

    int16_t x, y;
    while (!watch->getTouch(x, y)) {} // Wait for touch
    while (watch->getTouch(x, y)) {}  // Wait for release to exit
    //Clear screen
    watch->tft->fillScreen(TFT_BLACK);
}

void appTouch() {

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


// prtTime will display the current selected time and highlight
// the current digit to be updated in yellow
void prtTime(byte digit) {
    RTC_Date tnow = watch->rtc->getDateTime();

    uint8_t pthh = tnow.hour;
    uint8_t ptmm = tnow.minute;

    watch->tft->fillRect(0, 0, 100, 34, TFT_BLACK);
    if (digit == 1)   watch->tft->setTextColor(TFT_YELLOW);
    else   watch->tft->setTextColor(TFT_WHITE);
    watch->tft->drawNumber(int(pthh / 10), 5, 5, 2);
    if (digit == 2)   watch->tft->setTextColor(TFT_YELLOW);
    else   watch->tft->setTextColor(TFT_WHITE);
    watch->tft->drawNumber(pthh % 10, 25, 5, 2);
    watch->tft->setTextColor(TFT_WHITE);
    watch->tft->drawString(":",  45, 5, 2);
    if (digit == 3)   watch->tft->setTextColor(TFT_YELLOW);
    else   watch->tft->setTextColor(TFT_WHITE);
    watch->tft->drawNumber(int(ptmm / 10), 65 , 5, 2);
    if (digit == 4)   watch->tft->setTextColor(TFT_YELLOW);
    else   watch->tft->setTextColor(TFT_WHITE);
    watch->tft->drawNumber(ptmm % 10, 85, 5, 2);
}

// getTnum takes care of translating where we pressed into
// a number that was pressed. Returns -1 for no press
// and 13 for DONE
int getTnum() {
    int16_t x, y;
    if (!watch->getTouch(x, y)) return - 1;
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

void displayTime(boolean fullUpdate) {
    byte xpos = 40; // Stating position for the display
    byte ypos = 90;

    // Get the current data
    RTC_Date tnow = watch->rtc->getDateTime();

    uint8_t dthh = tnow.hour;
    uint8_t  dtmm = tnow.minute;
    uint8_t  dtss = tnow.second;
    uint8_t dtdday = tnow.day;
    uint8_t dtmmonth = tnow.month;
    uint16_t dtyyear = tnow.year;

    watch->tft->setTextSize(1);

    if (fullUpdate) {
        // Font 7 is a 7-seg display but only contains
        // characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .

        watch->tft->setTextColor(0x39C4); // Set desired color
        watch->tft->drawString("88:88", xpos, ypos, 7);
        watch->tft->setTextColor(0xFBE0); // Orange

        if (dthh < 10) xpos += watch->tft->drawChar('0', xpos, ypos, 7);
        xpos += watch->tft->drawNumber(dthh, xpos, ypos, 7);
        xcolon = xpos + 3;
        xpos += watch->tft->drawChar(':', xcolon, ypos, 7);
        if (dtmm < 10) xpos += watch->tft->drawChar('0', xpos, ypos, 7);
        watch->tft->drawNumber(dtmm, xpos, ypos, 7);
    }

    if (dtss % 2) { // Toggle the colon every second
        watch->tft->setTextColor(0x39C4);
        xpos += watch->tft->drawChar(':', xcolon, ypos, 7);
        watch->tft->setTextColor(0xFBE0);
    } else {
        watch->tft->drawChar(':', xcolon, ypos, 7);
    }
    watch->tft->setTextSize(3);
    watch->tft->setCursor( 10, 210);

    watch->tft->print(dtmmonth);
    watch->tft->print("/");
    watch->tft->print(dtdday);
    watch->tft->print("/");
    watch->tft->print(dtyyear);
}



void setup() {
    Serial.begin(115200);
    watch = TTGOClass::getWatch();
    watch->begin();
    watch->lvgl_begin();
    rtc = watch->rtc;
    rtc->syncToSystem();
    // Use compile time
    rtc->check();

    watch->openBL();

    //Lower the brightness
    watch->bl->adjust(150);

//    //initSetup();
//    watch->begin();
//    watch->tft->setTextFont(1);
//    watch->tft->fillScreen(TFT_BLACK);
//    watch->tft->setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour
//    //Initialize lvgl
//    watch->lvgl_begin();
//
//    watch->rtc->check();
//
//    //Synchronize time to system time

//
//
    displayTime(true); // Our GUI to show the time
//    watch->openBL(); // Turn on the backlight
//
//
    lv_obj_t *img1 = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img1, &cat_png);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_color(&style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&style, LV_STATE_DEFAULT, &cat_font);

    g_data.hour = lv_label_create(img1, nullptr);
    lv_obj_add_style(g_data.hour, LV_OBJ_PART_MAIN, &style);
    lv_label_set_text(g_data.hour, "00");
    lv_obj_align(g_data.hour, img1, LV_ALIGN_IN_TOP_MID, 10, 30);

    g_data.minute = lv_label_create(img1, nullptr);
    lv_obj_add_style(g_data.minute, LV_OBJ_PART_MAIN, &style);
    lv_label_set_text(g_data.minute, "00");
    lv_obj_align(g_data.minute, g_data.hour, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    g_data.second = lv_label_create(img1, nullptr);
    lv_obj_add_style(g_data.second, LV_OBJ_PART_MAIN, &style);
    lv_label_set_text(g_data.second, "00");
    lv_obj_align(g_data.second, g_data.minute, LV_ALIGN_OUT_RIGHT_MID, 9, 0);

    lv_task_create([](lv_task_t *t) {
        RTC_Date curr_datetime = rtc->getDateTime();
        lv_label_set_text_fmt(g_data.second, "%02u", curr_datetime.second);
        lv_label_set_text_fmt(g_data.minute, "%02u", curr_datetime.minute);
        lv_label_set_text_fmt(g_data.hour, "%02u", curr_datetime.hour);

    }, 1000, LV_TASK_PRIO_MID, nullptr);
    // Set 20MHz operating speed to reduce power consumption
    //setCpuFrequencyMhz(20);

}

void loop() {
  if (targetTime < millis()) {
    targetTime = millis() + 1000;
    displayTime(ss == 0); // Call every second but only update time every minute
      lv_task_handler();
  }

  int16_t x, y;
  if (watch->getTouch(x, y)) {
    while (watch->getTouch(x, y)) {} // wait for user to release

    // This is where the app selected from the menu is launched
    // If you add an app, follow the variable update instructions
    // at the beginning of the menu code and then add a case
    // statement on to this switch to call your paticular
    // app routine.

    switch (modeMenu()) { // Call modeMenu. The return is the desired app number
          case 0: // Zero is the clock, just exit the switch
            break;
          case 1:
            appBattery();
            break;
          case 2:
            jSats();
            break;
          case 3:
            appAccel();
            break;
          case 4:
            appTouch();
            break;
/*          case 5:
            appSetTime();
            break;
          case 6:
            KT();
            break;*/
    }
    displayTime(true);
  }
}

void appSetTime() {
    // Get the current info
    RTC_Date tnow = watch->rtc->getDateTime();

    uint8_t sthh = tnow.hour;
    uint8_t stmm = tnow.minute;
    uint8_t stdday = tnow.day;
    uint8_t stmmonth = tnow.month;
    uint16_t styyear = tnow.year;

//Set up the interface buttons

    watch->tft->fillScreen(TFT_BLACK);
    watch->tft->fillRect(0, 35, 80, 50, TFT_BLUE);
    watch->tft->fillRect(161, 35, 78, 50, TFT_BLUE);
    watch->tft->fillRect(81, 85, 80, 50, TFT_BLUE);
    watch->tft->fillRect(0, 135, 80, 50, TFT_BLUE);
    watch->tft->fillRect(161, 135, 78, 50, TFT_BLUE);
    watch->tft->fillRect(0, 185, 80, 50, TFT_BLUE);
    watch->tft->setTextColor(TFT_GREEN);
    watch->tft->drawNumber(1, 30, 40, 2);
    watch->tft->drawNumber(2, 110, 40, 2);
    watch->tft->drawNumber(3, 190, 40, 2);
    watch->tft->drawNumber(4, 30, 90, 2);
    watch->tft->drawNumber(5, 110, 90, 2);
    watch->tft->drawNumber(6, 190, 90, 2);
    watch->tft->drawNumber(7, 30, 140, 2);
    watch->tft->drawNumber(8, 110, 140, 2);
    watch->tft->drawNumber(9, 190, 140, 2);
    watch->tft->drawNumber(0, 30, 190, 2);
    watch->tft->fillRoundRect(120, 200, 119, 39, 6, TFT_WHITE);
    watch->tft->setTextSize(2);
    watch->tft->setCursor(0, 0);

    watch->tft->setCursor(155, 210);
    watch->tft->setTextColor(TFT_BLACK);
    watch->tft->print("DONE");
    watch->tft->setTextColor(TFT_WHITE);
    int wl = 0; // Track the current number selected
    byte curnum = 1;  // Track which digit we are on

    prtTime(curnum); // Display the time for the current digit

    while (wl != 13) {
        wl = getTnum();
        if (wl != -1) {

            switch (curnum) {
                case 1:
                    hh = wl * 10 + sthh % 10;
                    break;
                case 2:
                    hh = int(sthh / 10) * 10 + wl;
                    break;
                case 3:
                    mm = wl * 10 + stmm % 10;
                    break;
                case 4:
                    mm = int(stmm / 10) * 10 + wl;
                    break;
            }
            while (getTnum() != -1) {}
            curnum += 1;
            prtTime(curnum);
            if (curnum > 4) wl = 13;
        }
    }
    while (getTnum() != -1) {}
    watch->rtc->setDateTime(styyear, stmmonth, stdday, hh, mm, 0);
    watch->tft->fillScreen(TFT_BLACK);
}

 void KT()
{

        lv_obj_t *img1 = lv_img_create(lv_scr_act(), NULL);
        lv_img_set_src(img1, &cat_png);
        lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);

        static lv_style_t style;
        lv_style_init(&style);
        lv_style_set_text_color(&style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_text_font(&style, LV_STATE_DEFAULT, &cat_font);

        g_data.hour = lv_label_create(img1, nullptr);
        lv_obj_add_style(g_data.hour, LV_OBJ_PART_MAIN, &style);

        lv_label_set_text(g_data.hour, "00");
        lv_obj_align(g_data.hour, img1, LV_ALIGN_IN_TOP_MID, 10, 30);

        g_data.minute = lv_label_create(img1, nullptr);
        lv_obj_add_style(g_data.minute, LV_OBJ_PART_MAIN, &style);
        lv_label_set_text(g_data.minute, "00");
        lv_obj_align(g_data.minute, g_data.hour, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

        g_data.second = lv_label_create(img1, nullptr);
        lv_obj_add_style(g_data.second, LV_OBJ_PART_MAIN, &style);
        lv_label_set_text(g_data.second, "00");
        lv_obj_align(g_data.second, g_data.minute, LV_ALIGN_OUT_RIGHT_MID, 9, 0);

        lv_task_create([](lv_task_t *t) {

            RTC_Date curr_datetime = rtc->getDateTime();
            lv_label_set_text_fmt(g_data.second, "%02u", curr_datetime.second);
            lv_label_set_text_fmt(g_data.minute, "%02u", curr_datetime.minute);
            lv_label_set_text_fmt(g_data.hour, "%02u", curr_datetime.hour);

        }, 1000, LV_TASK_PRIO_MID, nullptr);

    int16_t x, y;
    while (!watch->getTouch(x, y)) {} // Wait for touch
    while (watch->getTouch(x, y)) {}  // Wait for release to exit
    //Clear screen
    watch->tft->fillScreen(TFT_BLACK);
}
