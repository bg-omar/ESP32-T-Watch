#include "config.h"
#include "Functions.h"
#include "CastaWatchCommon.h"
#include "Alarms.h"

/***********************************************************************************************
 *   Main Clock Display
 *********************************************************************************************** */

//lv_obj_t *screen = lv_obj_create(NULL);
lv_obj_t *timeLabel;
lv_obj_t *dateLabel;
lv_obj_t *batLabel;
lv_obj_t *ampmLabel;


void initializeClock(lv_obj_t* parent){

    //lv_obj_set_style_bg_color(parent, lv_color_hex(0xFF0000), 0);

    timeLabel = lv_label_create(parent);
    lv_label_set_text(timeLabel, "00:00");
    lv_obj_set_style_text_align(timeLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -10);
    LV_FONT_DECLARE(dseg_14_reg_60);
    lv_obj_set_style_text_font(timeLabel, &dseg_14_reg_60, LV_STATE_DEFAULT);

    dateLabel = lv_label_create(parent);
    lv_label_set_text(dateLabel, "{date}");
    lv_obj_set_style_text_align(dateLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(dateLabel, LV_ALIGN_CENTER, 0, 45);
    lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_32, LV_STATE_DEFAULT);

    batLabel = lv_label_create(parent);
    lv_label_set_text(batLabel, "{date}");
    lv_obj_set_style_text_align(batLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(batLabel, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_text_font(batLabel, &lv_font_montserrat_22, LV_STATE_DEFAULT);
    lv_label_set_recolor(batLabel, true);

    ampmLabel = lv_label_create(parent);
    lv_label_set_text(ampmLabel, "");
    lv_obj_set_style_text_align(ampmLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(ampmLabel, LV_ALIGN_CENTER, 90, -60);
    lv_obj_set_style_text_font(ampmLabel, &lv_font_montserrat_22, LV_STATE_DEFAULT);

    ttgo->power->adc1Enable(AXP202_VBUS_CONNECT_IRQ | AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
}


void updateDisplay(){
// Get the current data
  RTC_Date tnow = ttgo->rtc->getDateTime();

  uint8_t hh = tnow.hour;
  uint8_t mm = tnow.minute;
  uint8_t ss = tnow.second;
  uint8_t dday = tnow.day;
  uint16_t mmonth = tnow.month;
  uint16_t yyear = tnow.year;

   const char * weekDay = "SUN";
   switch (weekday(yyear, mmonth, dday))
   {
   case 0:
       weekDay ="SUN";
       break;
   case 1:
       weekDay ="MON";
       break;
   case 2:
       weekDay ="TUE";
       break;
   case 3:
       weekDay ="WED";
       break;
   case 4:
       weekDay ="THU";
       break;
   case 5:
       weekDay ="FRI";
       break;
   case 6:
       weekDay ="SAT";
       break;
   
   default:
       break;
   }

    const char * dot = ":";
//    if (ss % 2 == 0){
//       dot = " ";
//    }
   const char * monthtext = "   ";
   switch (mmonth)
   {
   case 1:
       monthtext = "JAN";
       break;
   case 2:
       monthtext = "FEB";
       break;
   case 3:
       monthtext = "MAR";
       break;
   case 4:
       monthtext = "APR";
       break;
   case 5:
       monthtext = "MAY";
       break;
   case 6:
       monthtext = "JUN";
       break;
   case 7:
       monthtext = "JUL";
       break;
   case 8:
       monthtext = "AUG";
       break;
   case 9:
       monthtext = "SEP";
       break;
   case 10:
       monthtext = "OCT";
       break;
   case 11:
       monthtext = "NOV";
       break;
   case 12:
       monthtext = "DEC";
       break;
   
   default:
       break;
   }

    int hh_converted = hh;
    bool isPM = hh>=12;
    const char * txtampm = "  ";

    if (!use24HS){
        if (hh_converted>=12){
            hh_converted-=12;
            txtampm ="PM";
        } else {
            txtampm ="AM";
        }
        if (hh_converted==0) hh_converted=12;
    }

    if (use24HS){
        lv_label_set_text_fmt(timeLabel, "%02d%s%02d", hh_converted,dot,mm);
    } else {
        lv_label_set_text_fmt(timeLabel, "%2d%s%02d", hh_converted,dot,mm);
    }

    lv_label_set_text_fmt(dateLabel, "%s, %s %02d",weekDay, monthtext, dday);
    lv_label_set_text_fmt(ampmLabel, "%s", txtampm);

    int per = ttgo->power->getBattPercentage();
    const char * baticon = LV_SYMBOL_BATTERY_EMPTY;
    const char * color = "#FF0000"; //RED
    if (per>=25){
        baticon = LV_SYMBOL_BATTERY_1;
//        color = "#FF8C00"; //ORANGE
        color = "#FFFF00"; //YELLOW
    }
    if (per>=50){
        baticon = LV_SYMBOL_BATTERY_2;
        color = "#FFFF00"; //YELLOW
    }
    if (per>=75){
        baticon = LV_SYMBOL_BATTERY_3;
        color = "#00FF00"; //GREEN
    }
    if (per>95){
        baticon = LV_SYMBOL_BATTERY_FULL;
        color = "#00FF00"; //GREEN
    }

    const char * connected = LV_SYMBOL_USB;
    if (!ttgo->power->isVBUSPlug()){
        connected = "";
    }

    const char * charging = LV_SYMBOL_CHARGE;
    if (!ttgo->power->isChargeing()){
        charging = "";
    }

    const char * alarm =LV_SYMBOL_BELL;
    if (AlarmScheduled<0){
        alarm = "";
    }
    lv_label_set_text_fmt(batLabel, "%s %s %s %d%% %s %s",alarm, connected , charging , per , color , baticon);
}
