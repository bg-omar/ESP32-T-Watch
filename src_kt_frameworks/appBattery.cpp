//
// Created by mr on 10/23/2023.
//
#include "config.h"
#include "appBattery.h"


void appBattery::battery() {
    TTGOClass *ttgo = TTGOClass::getWatch();
    TFT_eSPI *tft;
    AXP20X_Class *power;

    tft = ttgo->tft;
    power = ttgo->power;

        // Turn on the battery adc to read the values
        power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

    // Some display setting
    tft->setTextFont(0);
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->drawString("BATT STATS",  35, 30, 2);
    tft->setTextColor(TFT_GREEN, TFT_BLACK);


    // get the values
    float vbus_v = ttgo->power->getVbusVoltage();
    float vbus_c = ttgo->power->getVbusCurrent();
    float batt_v = ttgo->power->getBattVoltage();
    int per = ttgo->power->getBattPercentage();

// Print the values
    tft->setCursor(0, 100);
    tft->print("Vbus: "); tft->print(vbus_v); tft->println(" mV");
    tft->setCursor(0, 130);
    tft->print("Vbus: "); tft->print(vbus_c); tft->println(" mA");
    tft->setCursor(0, 160);
    tft->print("BATT: "); tft->print(batt_v); tft->println(" mV");
    tft->setCursor(0, 190);
    tft->print("Per: "); tft->print(per); tft->println(" %");

    int16_t x, y;
    while (!ttgo->getTouch(x, y)) {
      // A simple clear screen will flash a bit
      tft->fillRect(0, 0, 210, 130, TFT_BLACK);
      tft->setTextFont(1);
      tft->setCursor(0, 0);
      tft->print("CHARGE STATUS: ");
      // You can use isVBUSPlug to check whether the USB connection is normal
      if (power->isVBUSPlug()) {
          tft->println("CONNECTED");

          // Get USB voltage
          tft->print("VBUS Voltage:");
          tft->print(power->getVbusVoltage());
          tft->println(" mV");

          // Get USB current
          tft->print("VBUS Current: ");
          tft->print(power->getVbusCurrent());
          tft->println(" mA");

      } else {

          tft->setTextColor(TFT_RED, TFT_BLACK);
          tft->println("DISCONNECTED");
          tft->setTextColor(TFT_GREEN, TFT_BLACK);
      }

      tft->println();

      tft->print("BATTERY ");
      // You can use isBatteryConnect() to check whether the battery is connected properly
      if (power->isBatteryConnect()) {
          tft->println("CONNECTED:");

          // Get battery voltage
          tft->print("BAT Voltage:");
          tft->print(power->getBattVoltage());
          tft->println(" mV");

          // To display the charging status, you must first discharge the battery,
          // and it is impossible to read the full charge when it is fully charged
          if (power->isChargeing()) {
              tft->print("Charge:");
              tft->print(power->getBattChargeCurrent());
              tft->println(" mA");
              tft->print("Full: ");
              tft->print(power->getBattPercentage());
              tft->println(" %");
          } else {
              // Show current consumption
              tft->print("Discharge:");
              tft->print(power->getBattDischargeCurrent());
              tft->println(" mA");
              tft->print("Power: ");
              tft->print(power->getBattPercentage());
              tft->println(" %");

          }
      } else {
          tft->setTextColor(TFT_RED, TFT_BLACK);
          tft->println("FAILURE:");
          tft->println("NOT DETECTED!");
          tft->setTextColor(TFT_GREEN, TFT_BLACK);
      }
      delay(1000);
    }
    // Wait for touch
    //while (!ttgo->getTouch(x, y)) {}
    //while (ttgo->getTouch(x, y)) {}  // Wait for release to exit
    //Clear screen
    ttgo->tft->fillScreen(TFT_BLACK);
}




