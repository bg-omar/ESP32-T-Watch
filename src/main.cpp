// An Arduino based framework for the Lilygo T-Watch 2020
// Much of the code is based on the sample apps for the
// T-watch that were written and copyrighted by Lewis He.
//(Copyright (c) 2019 lewis he)
#include <iostream>
#include "config.h"
#include <SPI.h>
#include <soc/rtc.h>
#include <Wire.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Ticker.h>
#include <ctime>
#include "watch_Battery.h"

#include "gui.h"
#include <SPIFFS.h>	// includes FS.h
#include <WiFi.h>
#include "secret.h"
#include <ctime>

#define DEFAULT_SCREEN_TIMEOUT 10 * 1000

enum
{
    Q_EVENT_BMA_INT,
    Q_EVENT_AXP_INT,
};
TTGOClass *watch = TTGOClass::getWatch();
PCF8563_Class *rtc;
AXP20X_Class *power = watch->power;
LV_IMG_DECLARE(kt_png);
LV_FONT_DECLARE(kt_font);

QueueHandle_t g_event_queue_handle = NULL;
EventGroupHandle_t g_event_group = NULL;
EventGroupHandle_t isr_group = NULL;
bool lenergy = false;

bool marioloper, ktloper;
bool tryNTPtime = true;

// Wifi variables
// The credetials are stored in src/secret.h file that doesnt need to be synched with the repo. The following format is used:
// #define SSID "MY_SSID"
// #define PASSWORD "MY_PASSWORD"
const char *ssid = SSID;
const char *ssid_passphrase = PASSWORD;

// NTP Settings : for more ionformations, https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/
const char *ntpServer = "pool.ntp.org"; // (for worlwide NTP server)
// const char* ntpServer = "europe.pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

Gui *gui;

typedef struct {
    lv_obj_t *hour;
    lv_obj_t *minute;
    lv_obj_t *second;
} str_datetime_t;


static str_datetime_t g_data;



byte xcolon = 0; // location of the colon

uint32_t targetTime = 0;       // for next 1-second display update
uint32_t clockUpTime = 0;      // track the time the clock is displayed

uint8_t hh,  mm , ss , dday ,mmonth;
uint16_t yyear; // Year is 16 bit int
void prtTime(byte) ;
int getTnum() ;

//TTGOClass *ttgo = TTGOClass::getWatch();
#define APP_TIME_ZONE   1 // I am East Coast in Daylight Savings

const int maxApp = 7; // number of apps
String appName[maxApp] = {"Clock", "Battery", "Jupiter", "Accel", "Touch", "Mario", "KT"}; // app names


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
    //    watch->begin();
    watch->tft->setTextFont(1);
    watch->tft->setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour
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

void initWakeupTriggers()
{
    BMA *sensor = watch->bma;
    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    // Configure the BMA423 accelerometer
    sensor->accelConfig(cfg);
    // Enable BMA423 accelerometer
    sensor->enableAccel();

    // Disable BMA423 isStepCounter feature
    sensor->enableFeature(BMA423_STEP_CNTR, false);
    // Enable BMA423 isTilt feature
    sensor->enableFeature(BMA423_TILT, true);
    // Enable BMA423 isDoubleClick feature
    sensor->enableFeature(BMA423_WAKEUP, true);

    // Reset steps
    sensor->resetStepCounter();

    // Turn off feature interrupt
    sensor->enableStepCountInterrupt();

    sensor->enableTiltInterrupt();
    // It corresponds to isDoubleClick interrupt
    sensor->enableWakeupInterrupt();
}

void low_energy()
{
    if (watch->bl->isOn())
    {
        xEventGroupSetBits(isr_group, WATCH_FLAG_SLEEP_MODE);
        watch->closeBL();
        //watch->stopLvglTick();
        watch->bma->enableStepCountInterrupt(false);
        watch->displaySleep();
        lenergy = true;
        // rtc_clk_cpu_freq_set(RTC_CPU_FREQ_2M);
        setCpuFrequencyMhz(20);
        Serial.println("ENTER IN LIGHT SLEEEP MODE");
        gpio_wakeup_enable((gpio_num_t)AXP202_INT, GPIO_INTR_LOW_LEVEL);
        gpio_wakeup_enable((gpio_num_t)BMA423_INT1, GPIO_INTR_HIGH_LEVEL);
        esp_sleep_enable_gpio_wakeup();
        esp_light_sleep_start();
    }
    else
    {
        //watch->startLvglTick();
        watch->displayWakeup();
        watch->rtc->syncToSystem();
        lv_disp_trig_activity(NULL);
        gui->updateTime();
        gui->updateBatteryLevel();
        gui->updateStepCounter(watch->bma->getCounter());
        gui->updateWakeupCount();
        gui->updateDate();
        watch->openBL();
        watch->bma->enableStepCountInterrupt();
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
    lv_img_set_src(img1, &kt_png);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_color(&style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&style, LV_STATE_DEFAULT, &kt_font);

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
}


void marioLoop(){

        bool rlst;
        uint8_t data;
        //! Fast response wake-up interrupt
        EventBits_t bits = xEventGroupGetBits(isr_group);
        if (bits & WATCH_FLAG_SLEEP_EXIT) {
            if (lenergy) {
                lenergy = false;
                // rtc_clk_cpu_freq_set(RTC_CPU_FREQ_160M);
                setCpuFrequencyMhz(160);
            }

            low_energy();

            if (bits & WATCH_FLAG_BMA_IRQ) {
                do {
                    rlst = watch->bma->readInterrupt();
                } while (!rlst);
                xEventGroupClearBits(isr_group, WATCH_FLAG_BMA_IRQ);
            }
            if (bits & WATCH_FLAG_AXP_IRQ) {
                watch->power->readIRQ();
                watch->power->clearIRQ();
                //TODO: Only accept axp power pek key short press
                xEventGroupClearBits(isr_group, WATCH_FLAG_AXP_IRQ);
            }
            xEventGroupClearBits(isr_group, WATCH_FLAG_SLEEP_EXIT);
            xEventGroupClearBits(isr_group, WATCH_FLAG_SLEEP_MODE);
        }
        if ((bits & WATCH_FLAG_SLEEP_MODE)) {
            //! No event processing after entering the information screen
            return;
        }

        //! Normal polling
        if (xQueueReceive(g_event_queue_handle, &data, 5 / portTICK_RATE_MS) == pdPASS) {
            switch (data) {
                case Q_EVENT_BMA_INT:
                    do {
                        rlst = watch->bma->readInterrupt();
                    } while (!rlst);

                    //! setp counter
                    if (watch->bma->isStepCounter()) {
                        custom_log("Stepcounter: %d\n", watch->bma->getCounter());
                        gui->updateStepCounter(watch->bma->getCounter());
                    }
                    break;
                case Q_EVENT_AXP_INT:
                    watch->power->readIRQ();
                    if (watch->power->isVbusPlugInIRQ()) {
                        Serial.println("Charging");
                    }
                    if (watch->power->isVbusRemoveIRQ()) {
                        //updateBatteryIcon(LV_ICON_CALCULATION);
                        Serial.println("Finished charging");
                    }
                    if (watch->power->isChargingDoneIRQ()) {
                        Serial.println("Full charged");
                    }
                    if (watch->power->isPEKShortPressIRQ()) {
                        Serial.println("PEK Short press");
                        custom_log("Current time: %s\n", watch->rtc->formatDateTime());
                        watch->power->clearIRQ();
                        low_energy();
                        return;
                    }
                    watch->power->clearIRQ();
                    break;
                default:
                    break;
            }
        }

        if (watch->power->isVBUSPlug() || lv_disp_get_inactive_time(NULL) < DEFAULT_SCREEN_TIMEOUT) {
            lv_task_handler();
        } else {
            low_energy();
        }
}

bool syncRtc2Ntp()
{
    //connect to WiFi
    custom_log("Connecting to %s\n", ssid);
    WiFi.begin(ssid, ssid_passphrase);
    // after 6 sec if WiFi is not found abort and avoid locking the setup
    int timeoutMs = 6000;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        timeoutMs -= 500;
        Serial.print(".");
        if (timeoutMs <= 0)
        {
            custom_log("\nWifi connection timed-out!\n");
            WiFi.mode(WIFI_OFF);
            return false;
        }
    }
    custom_log("\nConnected to %s \n", ssid);

    //init and get the time from NTP server
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return false;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");


    RTC_Date updateRTC;
    watch->rtc->setDateTime(updateRTC);
    updateRTC.year = timeinfo.tm_year;
    updateRTC.month = timeinfo.tm_mon;
    updateRTC.day = timeinfo.tm_mday;
    updateRTC.hour = timeinfo.tm_hour-1;
    updateRTC.minute = timeinfo.tm_min;
    updateRTC.second = timeinfo.tm_sec;
    watch->rtc->setDateTime(updateRTC);

    custom_log("RTC time synched with NTP %2d.%2d.%4d\n", updateRTC.day, updateRTC.month, updateRTC.year);
    custom_log("Time now:  %2d.%2d.%4d\n", updateRTC.hour, updateRTC.minute, updateRTC.second);

    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return true;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Woked-up!");
    watch = TTGOClass::getWatch();

    initWakeupTriggers();

    Serial.println("Done Init!");
    //Create a program that allows the required message objects and group flags
    g_event_queue_handle = xQueueCreate(20, sizeof(uint8_t));
    g_event_group = xEventGroupCreate();
    isr_group = xEventGroupCreate();

    //Initialize TWatch
    watch->begin();
    watch->openBL();

    // Turn on the IRQ used
    watch->power->adc1Enable(AXP202_BATT_VOL_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1, AXP202_ON);
    watch->power->enableIRQ(AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_FINISHED_IRQ, AXP202_ON);
    watch->power->clearIRQ();


    // Turn off unused power
    // according to https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/docs/watch_2020_v2.md
    // TFT/TOUCHT
    watch->power->setPowerOutPut(AXP202_LDO3, AXP202_OFF);
    // GPS Module
    watch->power->setPowerOutPut(AXP202_LDO4, AXP202_OFF);


    // Enable BMA423 interrupt ，
    // The default interrupt configuration,
    // you need to set the acceleration parameters, please refer to the BMA423_Accel example
    watch->bma->attachInterrupt();
    //Connection interrupted to the specified pin
    pinMode(BMA423_INT1, INPUT);
    attachInterrupt(
            BMA423_INT1, [] {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                EventBits_t bits = xEventGroupGetBitsFromISR(isr_group);
                if (bits & WATCH_FLAG_SLEEP_MODE)
                {
                    //! For quick wake up, use the group flag
                    xEventGroupSetBitsFromISR(isr_group, WATCH_FLAG_SLEEP_EXIT | WATCH_FLAG_BMA_IRQ, &xHigherPriorityTaskWoken);
                }
                else
                {
                    uint8_t data = Q_EVENT_BMA_INT;
                    xQueueSendFromISR(g_event_queue_handle, &data, &xHigherPriorityTaskWoken);
                }

                if (xHigherPriorityTaskWoken)
                {
                    portYIELD_FROM_ISR();
                }
            },
            RISING);

    // Connection interrupted to the specified pin
    pinMode(AXP202_INT, INPUT);
    attachInterrupt(
            AXP202_INT, [] {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                EventBits_t bits = xEventGroupGetBitsFromISR(isr_group);
                if (bits & WATCH_FLAG_SLEEP_MODE)
                {
                    //! For quick wake up, use the group flag
                    xEventGroupSetBitsFromISR(isr_group, WATCH_FLAG_SLEEP_EXIT | WATCH_FLAG_AXP_IRQ, &xHigherPriorityTaskWoken);
                }
                else
                {
                    uint8_t data = Q_EVENT_AXP_INT;
                    xQueueSendFromISR(g_event_queue_handle, &data, &xHigherPriorityTaskWoken);
                }
                if (xHigherPriorityTaskWoken)
                {
                    portYIELD_FROM_ISR();
                }
            },
            FALLING);
    watch->lvgl_begin();
    rtc = watch->rtc;
    //Check if the RTC clock matches, if not, use compile time
    watch->rtc->check();
    custom_log("RTC time: %s\n", watch->rtc->formatDateTime());

    if (tryNTPtime)
    {
        //Synchronize time to NTP
        syncRtc2Ntp();
    }

    watch->rtc->syncToSystem();
    custom_log("RTC time: %s\n", watch->rtc->formatDateTime());

    gui = new Gui(new AbstractDevice());
    //gui->setupGui();

    //watch = TTGOClass::getWatch();
    //watch->begin();
    //watch->lvgl_begin();
    //rtc->syncToSystem();
    // Use compile time
    //rtc->check();
    //watch->openBL();
    //Lower the brightness
    //watch->bl->adjust(200);
    //initSetup();
    //Initialize lvgl
    //watch->lvgl_begin();
    //watch->rtc->check();
    //Synchronize time to system time
    //displayTime(true); // Our GUI to show the time
    //watch->openBL(); // Turn on the backlight

    // Set 20MHz operating speed to reduce power consumption
    //setCpuFrequencyMhz(20);

    marioloper = false;
    ktloper = false;
}


void loop()
{
    if (marioloper) {
        int16_t x, y;
        while (!watch->getTouch(x, y)) { marioLoop(); }// Wait for touch}
        while (watch->getTouch(x, y)) {}
        marioloper = false;
        custom_log(" ---> marioloper: %4d \n", marioloper); // Wait for release to exit
    }

    if (ktloper) {
        int16_t x, y;
        while (!watch->getTouch(x, y)) { lv_task_handler(); }// Wait for touch}
        while (watch->getTouch(x, y)) {}
        ktloper = false;
        custom_log(" ---> ktloper: %4d \n", ktloper); // Wait for release to exit
    }

    if (targetTime < millis()) {
        targetTime = millis() + 1000;
        displayTime(ss == 0); // Call every second but only update time every minute
    }

    int16_t x, y;
    if (watch->getTouch(x, y)) {
        while (watch->getTouch(x, y)) {} // wait for user to release
        marioloper = false;
        ktloper = false;

        switch (modeMenu()) { // Call modeMenu. The return is the desired app number
            case 0: // Zero is the clock, just exit the switch
                displayTime(true);
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
            case 5:
                marioloper = true;
                custom_log(" ---> marioloper: %4d\n", marioloper);
                gui->setupGui();
                marioLoop();
                break;
            case 6:
                ktloper = true;
                custom_log(" ---> ktloper: %4d\n", ktloper);
                watch->tft->fillScreen(TFT_BLACK);
                KT();
                break;
        }

    }
}


