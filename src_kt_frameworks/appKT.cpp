//
// Created by mr on 11/18/2023.
//

#include "appKT.h"

typedef struct {
    lv_obj_t *hour;
    lv_obj_t *minute;
    lv_obj_t *second;
} str_datetime_t;


static str_datetime_t g_data;

TTGOClass *watchKT = TTGOClass::getWatch();
PCF8563_Class *appKT::rtcKT;

void appKT::setupKT(){
    watchKT = TTGOClass::getWatch();
    watchKT->begin();
    watchKT->lvgl_begin();
    rtcKT = watchKT->rtc;

    // Use compile time
    rtcKT->check();

    watchKT->openBL();

    //Lower the brightness
    watchKT->bl->adjust(150);
}

void appKT::KT()
{
    lv_obj_t *img1 = lv_img_create(lv_scr_act(), nullptr);
    lv_img_set_src(img1, &kt_png);
    lv_obj_align(img1,nullptr, LV_ALIGN_CENTER, 0, 0);

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

        RTC_Date curr_datetime = rtcKT->getDateTime();
        lv_label_set_text_fmt(g_data.second, "%02u", curr_datetime.second);
        lv_label_set_text_fmt(g_data.minute, "%02u", curr_datetime.minute);
        lv_label_set_text_fmt(g_data.hour, "%02u", curr_datetime.hour);

    }, 1000, LV_TASK_PRIO_MID, nullptr);
}