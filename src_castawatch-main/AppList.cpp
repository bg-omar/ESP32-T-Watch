#include "config.h"
#include "Functions.h"
#include "CastaWatchCommon.h"
#include <ctype.h>
#include "AppList.h"

#include "SetDateTime.h"
#include "GeneralSettingsDisplay.h"
#include "Vumeter.h"
#include "SetAlarm.h"

static lv_obj_t * list1;
lv_obj_t * btn1;
lv_obj_t * btn2;
lv_obj_t * btn3;
lv_obj_t * btn4;
lv_obj_t * btn5;
lv_obj_t * btn6;


static void event_handler_btn1(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        lv_obj_t * target = lv_event_get_target(e);
        String txt = lv_list_get_btn_text(list1, target);
//        Serial.println("CLICK! " + txt);

        // Set DateTime
        if (target == btn1){
            LaunchSetDate();
        }
        // General Settings
        if (target == btn2){
            LaunchGeneralSettings();
        }

        //Set Alarms
        if (target == btn5){
            LaunchSetAlarm();
        }

        //vumeter
        if (target == btn6){
            LaunchVumeter();
        }

    }
}


void initializeAppList(lv_obj_t* parent){
    //  lv_obj_set_layout(parent, LV_LAYOUT_FLEX );
      
    list1 = lv_list_create(parent);
    lv_obj_set_size(list1, 240, 240);
    lv_obj_set_style_text_font(list1, &lv_font_montserrat_22, LV_STATE_DEFAULT);
    lv_obj_center(list1);
    lv_obj_set_scroll_dir(list1, LV_DIR_VER);

    btn1=lv_list_add_btn(list1,LV_SYMBOL_SETTINGS, "Set Date / Time");
    lv_obj_add_event_cb(btn1, event_handler_btn1, LV_EVENT_CLICKED, NULL);
    btn2=lv_list_add_btn(list1,LV_SYMBOL_SETTINGS, "Settings");
    lv_obj_add_event_cb(btn2, event_handler_btn1, LV_EVENT_CLICKED, NULL);
    btn3=lv_list_add_btn(list1,LV_SYMBOL_BLUETOOTH, "BLE");
    lv_obj_add_event_cb(btn3, event_handler_btn1, LV_EVENT_CLICKED, NULL);
    btn4=lv_list_add_btn(list1,LV_SYMBOL_WIFI, "WIFI");
    lv_obj_add_event_cb(btn4, event_handler_btn1, LV_EVENT_CLICKED, NULL);
    btn5=lv_list_add_btn(list1,LV_SYMBOL_BELL, "Alarms");
    lv_obj_add_event_cb(btn5, event_handler_btn1, LV_EVENT_CLICKED, NULL);
    btn6=lv_list_add_btn(list1,LV_SYMBOL_AUDIO, "Vumeter");
    lv_obj_add_event_cb(btn6, event_handler_btn1, LV_EVENT_CLICKED, NULL);
}
