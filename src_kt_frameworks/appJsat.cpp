//
// Created by mr on 11/18/2023.
//
#include "config.h"
#include "appJsat.h"


void appJsat::jSats() {
    TTGOClass *watch = TTGOClass::getWatch();
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


