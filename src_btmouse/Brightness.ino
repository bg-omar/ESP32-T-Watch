/*
T-WATCH-V2-Set-Time
        A simple sketch to set time with the Touch Screen to the T-Watch. Including Battery info and setting the Brightness.

Set hour: Touch the lower-right area of the screen.
Set Brightness: Touch the lower-left area of the screen. Brightness begins in 20 and increments by 50 until 170, then goes to 0 (Brightnes Off) and again to 20. But you can edit these values. When it´s 0, skips the principal loop´s code. When brightness at 0 (screen Off), touch the screen for 1 second until screen lights on again. Although brightness is at 0, code is running. If you want to switch off the watch, press the lateral button for 6 seconds.
NOTICE:

This sketch requires TTGO TWatch Library.
Remember select at the beginning, the correct version of your TWATCH (line 2).
I´m an amateur, and I´m sure the code can be improved a lot. These are my first steps with the T-WATCH, but it runs.*/

////////////////////////////////////////////////////////

// sep-2022 - Alberto Mouriño Pardo.

////////////////////////////////////////////////////////


#define LILYGO_WATCH_2020_V2
#include <LilyGoWatch.h>

TTGOClass *ttgo;
AXP20X_Class *axp;

int brillo = 20;
int contador = 0;
unsigned long tiempo_touch = millis();
bool estado_touch = false;
int16_t x = 0;
int16_t y = 0;
char buf[128];
char hora_minuto[7];
int hora = 0;
int minuto = 0;



/////////////////  FUNCIONES  /////////////////


void pantalla_ppal() {
    ttgo->tft->setTextColor(TFT_GREEN, TFT_BLACK);
    ttgo->tft->drawString("UP THE IRONS!", 5, 5, 2);
    ttgo->tft->drawString("Brillo:",  10, 225, 2);
    sprintf(buf, "%03d", brillo);
    ttgo->tft->drawString(buf, 55, 225, 2);
    ttgo->tft->fillRoundRect(159, 218, 80, 19, 8, TFT_BLUE);
    ttgo->tft->setTextColor(TFT_GREEN, TFT_BLUE);
    ttgo->tft->drawString("Ajusta hora",  164, 219, 2);
}

void bateria() {
    bool bat_cien = false;
    int nivel_bateria = axp->getBattPercentage();
    if (nivel_bateria == 100) bat_cien = true;
    if (nivel_bateria < 100 && bat_cien) {
        ttgo->tft->fillRoundRect(194, 5, 41, 16, 4, TFT_BLACK);
        bat_cien = false;
    }
    ttgo->tft->drawRoundRect(192, 3, 45, 20, 4, TFT_GREEN);
    ttgo->tft->drawRoundRect(193, 4, 43, 18, 4, TFT_GREEN);
    ttgo->tft->fillRect(236, 7, 5, 12, TFT_GREEN);
    ttgo->tft->setTextColor(TFT_GREEN, TFT_BLACK);
    ttgo->tft->drawString(String(nivel_bateria) + "%", 197, 5, 2);
    if (axp->getBattPercentage() == 100) bat_cien = true;
    ttgo->tft->drawString(String(axp->getBattVoltage() / 1000) + " v", 195, 25, 2);
}


void limpia_touch() {
    estado_touch = false;
    x = 0;
    y = 0;
}


////////////////////


void proc_brillo() {
    if (brillo >= 170) {
        brillo = 0;
    }
    else {
        if (brillo == 0) brillo = 20;
        else  {
            brillo += 50;
        }
        sprintf(buf, "%03d", brillo);
        ttgo->tft->drawString(buf, 55, 225, 2);
    }
    ttgo->setBrightness(brillo);
    Serial.println("Brillo: " + String(brillo));
    if (brillo == 0) delay(500);
}
////////////////////


void proc_poner_en_hora() {
    bool confirmado = false;
    unsigned long  tmp_confirmacion = millis();
    int contador = 0;
    ttgo->tft->fillRoundRect(159, 218, 80, 19, 8, TFT_BLACK);
    ttgo->tft->fillRoundRect(73, 110, 100, 19, 8, TFT_BLUE);
    ttgo->tft->setTextColor(TFT_GREEN, TFT_BLUE);
    ttgo->tft->drawString("¿Cambiar hora?",  79, 111, 2);
    ttgo->tft->fillRoundRect(52, 150, 50, 22, 8, TFT_BLUE);
    ttgo->tft->fillRoundRect(132, 150, 50, 22, 8, TFT_BLUE);
    ttgo->tft->drawString("SI",  70, 152, 2);
    ttgo->tft->drawString("NO",  150, 152, 2);

    while (1) {
        estado_touch = ttgo->getTouch(x, y);
        if (x >= 50 && x <= 100 && y >= 150 && y <= 200) {
            confirmado = true;
            break;
        }
        if (x >= 130 && x <= 180 && y >= 150 && y <= 200) {
            break;
        }
        if (millis() > tmp_confirmacion + 1000) {
            tmp_confirmacion = millis();
            contador++;
        }
        if (contador == 3) {
            break;
        }
    }
    ttgo->tft->fillRect(50, 110, 135, 70, TFT_BLACK);
    if (confirmado) {
        ttgo->tft->setTextColor(TFT_RED, TFT_BLACK);
        String fecha_hora = String(ttgo->rtc->formatDateTime());
        Serial.println(fecha_hora);
        String hora_ = fecha_hora.substring(0, 2);
        String minuto_ = fecha_hora.substring(3, 5);
        sprintf(hora_minuto, "%02s:%02s", hora_.c_str(), minuto_.c_str());
        Serial.println(hora_);
        Serial.println(minuto_);
        Serial.println(hora_minuto);
        ttgo->tft->fillRect(145, 40, 94, 50, TFT_BLACK);
        ttgo->tft->drawString(hora_minuto,  15, 105, 2);
        ttgo->tft->setTextColor(TFT_GREEN, TFT_BLUE);
        ttgo->tft->fillRect(10, 125, 80, 50, TFT_BLUE);
        ttgo->tft->drawString("Hora +",  30, 140, 2);
        ttgo->tft->fillRect(10, 189, 80, 50, TFT_BLUE);
        ttgo->tft->drawString("Hora -",  30, 204, 2);
        ttgo->tft->fillRect(159, 125, 80, 50, TFT_BLUE);
        ttgo->tft->drawString("Minuto +",  174, 140, 2);
        ttgo->tft->fillRect(159, 189, 80, 50, TFT_BLUE);
        ttgo->tft->drawString("Minuto -",  174, 204, 2);
        int timer = 4;

        while (1) {
            estado_touch = false;
            estado_touch = ttgo->getTouch(x, y);
            if (!estado_touch) {
                if (millis() > tiempo_touch + 1000) {
                    tiempo_touch = millis();
                    timer--;
                    if (timer == 0) {
                        ttgo->rtc->setDateTime(2022, 9, 7, hora, minuto, 0);
                        ttgo->tft->fillRect(10, 125, 229, 114, TFT_BLACK);
                        Serial.println("No ajustaste.");
                        break;
                    }
                }
            }
            else {
                timer = 4;
                if (x <= 79 && y >= 125 && y < 174) {
                    hora++;
                    if (hora > 23) hora = 0;
                }
                if (x <= 79 && y >= 189) {
                    hora--;
                    if (hora < 0) hora = 23;
                }
                if (x >= 189 && y >= 125 && y < 174) {
                    minuto++;
                    if (minuto > 59) minuto = 0;
                }
                if (x >= 189 && y >= 189) {
                    minuto--;
                    if (minuto < 0) minuto = 59;
                }
                ttgo->tft->setTextColor(TFT_RED, TFT_BLACK);
                sprintf(hora_minuto, "%02d:%02d", hora, minuto);
                ttgo->tft->drawString(hora_minuto,  15, 40, 7);
                Serial.println("Hora: " + String(hora));
                Serial.println("Minuto: " + String(minuto));
                delay(500);
            }
        }
    }
}

void captura_touch() {
    limpia_touch();
    estado_touch = ttgo->getTouch(x, y);
    if (estado_touch && brillo == 0) {
        brillo = 20;
        ttgo->setBrightness(brillo);
        limpia_touch();
    }
    if (x < 25 && y > 215) {
        proc_brillo();
        limpia_touch();
    }
    if (x > 159 && y > 195) {
        proc_poner_en_hora();
        limpia_touch();
    }
}

////////////////////

void setup() {
    Serial.begin(115200);
    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    axp = ttgo->power;
    axp->adc1Enable(
            AXP202_VBUS_VOL_ADC1 |
            AXP202_VBUS_CUR_ADC1 |
            AXP202_BATT_CUR_ADC1 |
            AXP202_BATT_VOL_ADC1,
            true);
    ttgo->openBL();
    ttgo->setBrightness(brillo);
    ttgo->tft->fillScreen(TFT_BLACK);
    pantalla_ppal();
    snprintf(buf, sizeof(buf), "%s", ttgo->rtc->formatDateTime());
    ttgo->tft->drawString(buf, 15, 40, 7);
}


void loop() {
    captura_touch();
    if (brillo != 0) {
        ttgo->tft->setTextColor(TFT_GREEN, TFT_BLACK);
        snprintf(buf, sizeof(buf), "%s", ttgo->rtc->formatDateTime());
        ttgo->tft->drawString(buf, 15, 40, 7);
        pantalla_ppal();
        bateria();
        delay(485);
        x = 0;
        y = 0;
        //    Serial.println("corriendo");
    }
    else {
        delay(1000);
    }
    //  Serial.println("Fin loop.\n");
}
