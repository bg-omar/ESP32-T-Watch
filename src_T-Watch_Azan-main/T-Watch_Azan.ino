#include "config.h"
#include "PrayerTimes.h"     
#include "DateHelper.h"     

// RGB565 Colors
#define BLACK       0x0000
#define DARK_GREY   0x10A2
#define GREEN       0x07E0
#define ORANGE      0xFDA0
#define RED         0xF800
#define SILVER      0xC618
#define GOLD        0xFEA0

TTGOClass           *watch  = NULL;
byte                counterToPowOff = 1;
bool                irq = false, goToSleep = false;
String              currentDateTime, PrayerHour, PrayerMinute;
String              year, month, date, minute, hour;       
String              prayerNames[6] = {"Fajr", "Shurooq","Duhr","Asr","Maghrib","Isha"};
String              daysOfWeek[7]  = {"Mon.", "Tue.", "Wed.", "Thu.", "Fri.", "Sat.", "Sun."};
uint8_t             IqamaOffset[6] = {30,10,10,10,5,10}, highlighted = -1;
uint16_t            currentAbsMinute;
int16_t             TodaysPrayers[6], dayOfYear, dayOfWeek, elapsedMinutes, minutesToNext;
int16_t             xTouch = 0, yTouch = 0, batteryPct;


void showEverything()
{
     dayOfYear = getDayOfYear((date).toInt(), (month).toInt(), (year).toInt());
     dayOfWeek = getWeekDay((date).toInt(), (month).toInt(), (year).toInt());
     highlighted = -1;

     // Loading prayers of the day
     for(int i=0; i<6; i++)
          TodaysPrayers[i] = PrayerTimes[dayOfYear][i];
          
     for(int i=0; i<6; i++){
          watch->tft->setTextColor(DARK_GREY, BLACK);   // FOREGROUND, BACKGROUND
          PrayerHour     =    String(TodaysPrayers[i]/60);
          PrayerHour     =    PrayerHour.length() > 1?PrayerHour : "0"+PrayerHour;
          PrayerMinute   =    String(TodaysPrayers[i]%60);
          PrayerMinute   =    PrayerMinute.length() > 1?PrayerMinute : "0"+PrayerMinute;
          elapsedMinutes =    currentAbsMinute-TodaysPrayers[i];
          minutesToNext  =    currentAbsMinute-TodaysPrayers[(i+1)%6];

          if(elapsedMinutes >= -15 && elapsedMinutes < 0) {      // Gold 15 min before prayer time
               watch->tft->setTextColor(GOLD, BLACK);
               watch->motor_begin();
               watch->shake();     
               highlighted = i;          
          } 
          else if(elapsedMinutes >= 0 && elapsedMinutes <= IqamaOffset[i]){     // Silver between azan and iqama
               watch->tft->setTextColor(SILVER, BLACK);
               goToSleep = true;
               highlighted = i; 
          }
          else if(elapsedMinutes > IqamaOffset[i] && elapsedMinutes <= 60){     // Green for 1 hour after the azan     
               watch->tft->setTextColor(GREEN, BLACK);
               highlighted = i;
          }
          else if(elapsedMinutes > 60 && (i == 5 || minutesToNext < -15)){      // Orange until next prayer or until midnight for Isha                               
               watch->tft->setTextColor(ORANGE, BLACK);
               highlighted = i;
          }

          watch->tft->drawString(prayerNames[i],0,35*i);
          watch->tft->drawString(PrayerHour + ":" + PrayerMinute,150,35*i);
     }

     // Adaptive brightness
     switch(highlighted) {
          case 0: watch->setBrightness(64);  break;
          case 1: watch->setBrightness(128); break;
          case 2: watch->setBrightness(255); break;
          case 3: watch->setBrightness(222); break;  
          case 4: watch->setBrightness(128); break;
          case 5: watch->setBrightness(64);  break;
     }

     // watch->tft->setFreeFont(&FreeSans12pt7b);    
     watch->tft->setTextColor(DARK_GREY, BLACK);                      
     watch->tft->drawString(hour+":"+minute,150,35*6);

     batteryPct = watch->power->getBattPercentage();
     if(watch->power->isVBUSPlug()) {
          if(batteryPct > 99)
               watch->tft->setTextColor(GREEN, BLACK);                      
          else
               watch->tft->setTextColor(ORANGE, BLACK);                      
          watch->tft->drawString(String(batteryPct)+"%",0,35*6);
          delay(3001);
     } else if(batteryPct < 33) {
          watch->tft->setTextColor(RED, BLACK);                      
          watch->tft->drawString(String(batteryPct)+"%",0,35*6);
          delay(1501);
     }   
}

void refreshTime()
{
     currentDateTime = watch->rtc->formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S); 
     year      = currentDateTime.substring(0,4);
     month     = currentDateTime.substring(5,7);
     date      = currentDateTime.substring(8,10);
     hour      = currentDateTime.substring(11,13);
     minute    = currentDateTime.substring(14,16);

     month     = month.length()>1?month:"0"+month;
     date      = date.length()>1?date:"0"+date;
     hour      = hour.length()>1?hour:"0"+hour;
     minute    = minute.length()>1?minute:"0"+minute;
     currentAbsMinute = (hour).toInt()*60+(minute).toInt();
}

void setup()
{
     //Serial.begin(115200);
     watch = TTGOClass::getWatch();
     watch->begin();
     watch->openBL();

     watch->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | 
     AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | 
     AXP202_BATT_VOL_ADC1, true);  

     pinMode(AXP202_INT, INPUT_PULLUP);
     attachInterrupt(AXP202_INT, [] {irq = true;}, FALLING);
    
     watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
     watch->power->clearIRQ();

     watch->tft->setTextSize(1);    
     watch->tft->setFreeFont(&FreeSans18pt7b);                           
     
     refreshTime();     
     showEverything();
}

void wakeUp()
{
     watch->displayWakeup();
     watch->openBL();
}

void lightPowerOff()
{
     goToSleep = false;
     watch->displaySleep();
     watch->closeBL();  
}

// shut off the watch completly
void powerOff()
{
     lightPowerOff();
     watch->powerOff();
     esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
     esp_deep_sleep_start();  
}

void loop()
{
     refreshTime();

     if(watch->bl->isOn()){
          watch->tft->setTextColor(DARK_GREY, BLACK);                      
          watch->tft->drawString(hour+(counterToPowOff%2?':':' ')+minute,150,35*6);
     
          if(dayOfWeek == 4)  // It's Friday!
               watch->tft->setTextColor(GOLD, BLACK);                      

          if(counterToPowOff < 3)
               watch->tft->drawString(daysOfWeek[dayOfWeek]+"   ",0,35*6);
          else if(counterToPowOff < 6)
               watch->tft->drawString(date+"-"+month+"   ",0,35*6);
     } else {
          for(int p=0; p<6; p++)
               if(currentAbsMinute == TodaysPrayers[p]) {
                    watch->motor_begin();
                    watch->shake();
                    wakeUp();
                    break;
               }
     }

     delay(1000);       

     if(irq) { // Poweroff on button press
          irq = false;
          watch->power->readIRQ();
          if (watch->power->isPEKShortPressIRQ()) {
               watch->power->clearIRQ();
               if(watch->bl->isOn())
                    counterToPowOff = 9;    // Turn off
               else   
                    counterToPowOff = -1;    // Turn on
          }
     watch->power->clearIRQ();
     }
     
     // Reset sleep timer if screen touched
     if(watch->getTouch(xTouch, yTouch)) {
          counterToPowOff = xTouch = yTouch = 0;
     }
          
     // Put to sleep if time elapsed     
     if (++counterToPowOff > 9)    
          if(goToSleep) lightPowerOff(); else powerOff();
     else if(!counterToPowOff)               // Will become 0 if just woken up
               wakeUp();
}