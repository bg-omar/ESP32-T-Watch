#include "config.h"
#include "SetAlarm.h"
#include "CastaWatchCommon.h"
#include "Alarms.h"

static lv_obj_t * myScreen;
static lv_obj_t * myNewAlarm;
static lv_obj_t * rollerHour;
static lv_obj_t * rollerMin;
static lv_obj_t * list1;
static lv_obj_t * ddAlarmSound;
static lv_obj_t * swRepeat;

// Close alarms set
static void close(){
    
    lv_scr_load(mainScreen);
    lv_obj_del(myScreen);
    InitializeSpeaker();
}

// Gesture to close alarms
void event_close(lv_event_t * e)
{
    lv_dir_t gesture = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (gesture == LV_DIR_RIGHT){
        close();
    }
}

// close new alarm ui
void closenewalarm(){
    lv_scr_load(myScreen);
    lv_obj_del(myNewAlarm);
}

uint32_t selectedindex; //selected alarm index

static void event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_current_target(e);
    if (lv_msgbox_get_active_btn_text(obj)=="Yes"){
       DeleteAlarm(selectedindex);
       lv_msgbox_close(obj);
       rebuildlist();
    } else {
        lv_msgbox_close(obj);
    };
}


void event_alarmdelete(lv_event_t * e){
    lv_obj_t * target= e->target;
    selectedindex= lv_obj_get_index(target);

    static const char * btns[] = {"Yes", "No", ""};
    char *code = (char*)malloc(25 * sizeof(char));
    sprintf(code, "Delete the alarm %02d:%02d? ", Alarms[selectedindex].hour, Alarms[selectedindex].minute);

    lv_obj_t * mbox1 = lv_msgbox_create(NULL, "Delete Alarm", code, btns, false);
    lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(mbox1);

}

void rebuildlist(){

    lv_obj_clean(list1);

    if (AlarmCount==0){
        lv_obj_t * btnlist1 = lv_list_add_btn(list1, NULL, "No Alarms Configured");
        return;
    }

    for(int i=0;i<AlarmCount;i++){
        char *code = (char*)malloc(30 * sizeof(char));
        const char * repeat = "      ";
        if (Alarms[i].setting2>0){
            repeat = "Repeat";
        }
        sprintf(code, "%02d. %02d:%02d %s",i+1, Alarms[i].hour, Alarms[i].minute, repeat);
        lv_obj_t * btnlist = lv_list_add_btn(list1, NULL, code);
        lv_obj_add_event_cb(btnlist, event_alarmdelete, LV_EVENT_LONG_PRESSED, NULL );
    }    

}

void addnewalarm_event(lv_event_t * e){
    schedule_t newAlarm;
    newAlarm.hour = lv_roller_get_selected(rollerHour);
    newAlarm.minute = lv_roller_get_selected(rollerMin);
    newAlarm.setting1 = lv_dropdown_get_selected(ddAlarmSound);
    if (lv_obj_has_state(swRepeat, LV_STATE_CHECKED)){
        newAlarm.setting2 = 1;
    }
    AddNewAlarm(newAlarm);
    closenewalarm();
    Serial.printf("Alarms %d\n", AlarmCount);
    rebuildlist();
}

void cancelnewalarm_event(lv_event_t * e){
    closenewalarm();
}

void event_alarmsound(lv_event_t * e){
    if (mp3->isRunning()){
        mp3->stop();
    }
}

void event_testsound (lv_event_t * e){
   uint16_t index = lv_dropdown_get_selected(ddAlarmSound);
   PlaySound(index);
}

void event_newalarm(lv_event_t * e){

    if (AlarmCount>=10) return; // allows only 10 alarms

    myNewAlarm =  lv_obj_create(NULL);
    lv_obj_set_size(myNewAlarm, 240, 240);
    lv_scr_load(myNewAlarm);

    lv_obj_t * btn = lv_btn_create(myNewAlarm);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, 0,0);
    lv_obj_t * lbl = lv_label_create(btn);
    lv_obj_align(lbl,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(lbl, LV_SYMBOL_OK " OK");
    lv_obj_add_event_cb(btn, addnewalarm_event, LV_EVENT_CLICKED, NULL);
    lv_obj_set_width(btn, 120);

    btn = lv_btn_create(myNewAlarm);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 0,0);
    lbl = lv_label_create(btn);
    lv_obj_align(lbl,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(lbl, LV_SYMBOL_CLOSE " Close");
    lv_obj_add_event_cb(btn, cancelnewalarm_event, LV_EVENT_CLICKED, NULL);
    lv_obj_set_width(btn, 120);

    static lv_style_t style_sel;
    lv_style_init(&style_sel);
    lv_style_set_text_font(&style_sel, &lv_font_montserrat_28);

    rollerHour = lv_roller_create(myNewAlarm);
    rollerMin = lv_roller_create(myNewAlarm);

    lv_roller_set_options(rollerHour,
                          "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n"
                          "10\n11\n12\n13\n"
                          "14\n15\n16\n17\n"
                          "18\n19\n20\n21\n"
                          "22\n23"
                          ,
                          LV_ROLLER_MODE_NORMAL);

    lv_roller_set_options(rollerMin,
                          "00\n01\n02\n03\n"
                          "04\n05\n06\n07\n"
                          "08\n09\n10\n11\n"
                          "12\n13\n14\n15\n"
                          "16\n17\n18\n19\n"
                          "20\n21\n22\n23\n"
                          "24\n25\n26\n27\n"
                          "28\n29\n30\n31\n"
                          "32\n33\n34\n35\n"
                          "36\n37\n38\n39\n"
                          "40\n41\n42\n43\n"
                          "44\n45\n46\n47\n"
                          "48\n49\n50\n51\n"
                          "52\n53\n54\n55\n"
                          "56\n57\n58\n59",
                          LV_ROLLER_MODE_NORMAL);

    lv_roller_set_visible_row_count(rollerHour, 4);
    lv_roller_set_visible_row_count(rollerMin, 4);
    lv_obj_add_style(rollerHour, &style_sel, LV_PART_MAIN);
    lv_obj_add_style(rollerMin, &style_sel, LV_PART_MAIN);

    lv_coord_t vert = -51;
    lbl = lv_label_create(myNewAlarm);
    lv_obj_align(lbl,LV_ALIGN_CENTER,0,vert);
    lv_obj_add_style(lbl, &style_sel, LV_PART_MAIN);
    lv_label_set_text(lbl,":");

    lv_obj_align(rollerHour, LV_ALIGN_CENTER, -50,vert);
    lv_obj_align(rollerMin, LV_ALIGN_CENTER, 50,vert);

    RTC_Date tnow = ttgo->rtc->getDateTime();
    uint8_t hh = tnow.hour;
    uint8_t mm = tnow.minute;
    lv_roller_set_selected(rollerHour, hh, LV_ANIM_ON);
    lv_roller_set_selected(rollerMin, mm, LV_ANIM_ON);

    ddAlarmSound = lv_dropdown_create(myNewAlarm);
    lv_dropdown_set_options(ddAlarmSound, "Alarm 1\n"
            "Alarm 2\n"
            "Alarm 3\n"
            "Alarm 4\n"
            "Alarm 5"
            );
    lv_obj_align(ddAlarmSound, LV_ALIGN_CENTER, -40, 37 );
    lv_obj_add_event_cb(ddAlarmSound, event_alarmsound ,LV_EVENT_VALUE_CHANGED, NULL);

    btn = lv_btn_create(myNewAlarm);
    lv_obj_align(btn, LV_ALIGN_CENTER,70,37);
    lbl = lv_label_create(btn);
    lv_obj_align(lbl,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(lbl,"Play");
    lv_obj_add_event_cb(btn, event_testsound ,LV_EVENT_CLICKED, NULL);

    swRepeat = lv_switch_create(myNewAlarm);
    lv_obj_align(swRepeat, LV_ALIGN_CENTER, 30, 70);
    lbl = lv_label_create(myNewAlarm);
    lv_obj_align(lbl,LV_ALIGN_CENTER,-30,70);
    lv_label_set_text(lbl,"Repeat");

}

void LaunchSetAlarm(){
 
    myScreen =  lv_obj_create(NULL);
    lv_obj_set_size(myScreen, 240, 240);
    lv_scr_load(myScreen);
    lv_obj_add_event_cb(myScreen,event_close, LV_EVENT_GESTURE, NULL );

//    uint8_t status = ttgo->rtc->status2(); 
//    Serial.printf("%x \n", status);

    lv_obj_t * lbl;// = lv_label_create(myScreen);
//    lv_obj_align(myScreen, LV_ALIGN_TOP_LEFT, 0,0);

/*
    char *code = (char*)malloc(50 * sizeof(char));
    sprintf(code, "%x %d", status, status);
    lv_label_set_text(lbl, code);
*/

    lv_obj_t * btn = lv_btn_create(myScreen);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0,0);
    lv_obj_set_width(btn, 240);
    lbl = lv_label_create(btn);
    lv_obj_align(lbl,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(lbl, LV_SYMBOL_PLUS " New Alarm");
    lv_obj_add_event_cb(btn, event_newalarm, LV_EVENT_CLICKED, NULL);

    list1 = lv_list_create(myScreen);
    lv_obj_set_size(list1, 240, 200);
    lv_obj_set_style_text_font(list1, &lv_font_montserrat_22, LV_STATE_DEFAULT);
    lv_obj_align(list1, LV_ALIGN_TOP_MID,0,0);
    lv_obj_set_scroll_dir(list1, LV_DIR_VER);

    rebuildlist();

}

