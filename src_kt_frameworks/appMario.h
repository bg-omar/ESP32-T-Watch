//
// Created by mr on 11/18/2023.
//

#ifndef ARDUINO_KT_WATCH_APPMARIO_H
#define ARDUINO_KT_WATCH_APPMARIO_H

#include "config.h"
#include <WiFi.h>
#include "secret.h"
#include "gui.h"

#define DEFAULT_SCREEN_TIMEOUT 10 * 1000
enum
{
    Q_EVENT_BMA_INT,
    Q_EVENT_AXP_INT,
};
static const char *ntpServer = "pool.ntp.org"; // (for worlwide NTP server)

class appMario {
private:
    static bool tryNTPtime;
public:
    static Gui *gui;
    static const char *ssid;
    static const char *ssid_passphrase;
    static QueueHandle_t g_event_queue_handle;
    static EventGroupHandle_t g_event_group;
    static EventGroupHandle_t isr_group;
    static void setupGUI();

  //const char* ntpServer = "europe.pool.ntp.org";
    static const long gmtOffset_sec = 3600;
    static const int daylightOffset_sec = 3600;

    static bool lenergy;
    static void marioLoop();
    static void low_energy();
    static void setupMario();
    static void initWakeupTriggers();

    static bool syncRtc2Ntp();
};


#endif //ARDUINO_KT_WATCH_APPMARIO_H
