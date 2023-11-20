//
// Created by mr on 10/23/2023.
//
#include "config.h"
#include "appBattery.h"


void appBattery::battery() {
    TTGOClass *watch = TTGOClass::getWatch();
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




