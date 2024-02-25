#include "config.h"
#include "SetDateTime.h"
#include "CastaWatchCommon.h"

static lv_obj_t * myScreen;
static lv_obj_t * calendar;
static lv_obj_t * rollerHour;
static lv_obj_t * rollerMin;
bool isScrolling = false;

void LaunchSetDate(){

    myScreen =  lv_obj_create(NULL);
    lv_obj_set_size(myScreen, 240, 240);
    lv_scr_load(myScreen);    

    // create paged panel
    lv_obj_t * panel = lv_obj_create(myScreen);
    lv_obj_set_size(panel, 240, 240);
    lv_obj_set_scroll_snap_x(panel, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scroll_snap_y(panel, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_scroll_dir(panel, LV_DIR_HOR);
    lv_obj_set_style_pad_gap(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 0,0);
    lv_obj_add_event_cb(panel, event_scrolling, LV_EVENT_SCROLL_BEGIN , NULL);
    lv_obj_add_event_cb(panel, event_scrolling, LV_EVENT_SCROLL_END , NULL);

    static lv_style_t scrollbarstyle;
    lv_style_init(&scrollbarstyle);
    lv_style_set_width(&scrollbarstyle,0);
    lv_style_set_height(&scrollbarstyle,0);
    lv_obj_add_style(panel, &scrollbarstyle, LV_PART_SCROLLBAR);

    lv_obj_t * t1 = lv_obj_create(panel);
    lv_obj_set_size(t1, 240, 240);
    lv_obj_align(t1, LV_ALIGN_CENTER,0,0);
    lv_obj_set_style_pad_gap(t1, 0, 0);
    lv_obj_set_style_pad_all(t1, 0,0);

    // panel 2 -> time
    lv_obj_t * t2 = lv_obj_create(panel);
    lv_obj_set_size(t2, 240, 240);
    lv_obj_align(t2, LV_ALIGN_CENTER,0,0);
    lv_obj_set_style_pad_gap(t2, 0, 0);
    lv_obj_set_style_pad_all(t2, 0,0);

    // panel 1 -> calendar
    calendar = lv_calendar_create(t2);
    lv_obj_set_size(calendar, 240, 240);
    lv_obj_align(calendar, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(calendar, event_handler, LV_EVENT_ALL, NULL);

    static lv_style_t style_sel;
    lv_style_init(&style_sel);
    lv_style_set_text_font(&style_sel, &lv_font_montserrat_28);

    RTC_Date tnow = ttgo->rtc->getDateTime();

    uint8_t hh = tnow.hour;
    uint8_t mm = tnow.minute;
    uint8_t ss = tnow.second;
    uint8_t dday = tnow.day;
    uint16_t mmonth = tnow.month;
    uint16_t yyear = tnow.year;


#if LV_USE_CALENDAR_HEADER_DROPDOWN
    lv_calendar_header_dropdown_create(calendar);
#elif LV_USE_CALENDAR_HEADER_ARROW
    lv_calendar_header_arrow_create(calendar);
#endif
    lv_calendar_set_showed_date(calendar, yyear, mmonth);
    lv_calendar_set_today_date(calendar, yyear, mmonth, dday);



    rollerHour = lv_roller_create(t1);
    rollerMin = lv_roller_create(t1);



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

    lv_obj_align(rollerHour, LV_ALIGN_CENTER, -50,-30);
    lv_obj_align(rollerMin, LV_ALIGN_CENTER, 50,-30);

    static lv_style_t style_sel1;
    lv_style_init(&style_sel1);
    lv_style_set_text_font(&style_sel1, &lv_font_montserrat_22);
    lv_style_set_radius(&style_sel1, 50);


    lv_obj_t * ok = lv_btn_create(t1);
    lv_obj_align(ok,LV_ALIGN_BOTTOM_RIGHT, 0,0);
    lv_obj_t * lbl = lv_label_create(ok);
    lv_label_set_text(lbl,"OK");
    lv_obj_set_size(ok, 118,60);
    lv_obj_add_style(ok, &style_sel1, LV_PART_MAIN);
    lv_obj_center(lbl);

    lv_obj_t * cancel = lv_btn_create(t1);
    lv_obj_align(cancel,LV_ALIGN_BOTTOM_LEFT, 0,0);
    lbl = lv_label_create(cancel);
    lv_label_set_text(lbl,"CANCEL");
    lv_obj_set_size(cancel, 118,60);
    lv_obj_add_style(cancel, &style_sel1, LV_PART_MAIN);
    lv_obj_center(lbl);

    lv_obj_t * lblz = lv_label_create(t1);
    lv_label_set_text(lblz,":");
    lv_obj_add_style(lblz, &style_sel, LV_PART_MAIN);
    lv_obj_align(lblz, LV_ALIGN_CENTER, 0,-30);

    lv_roller_set_selected(rollerHour, hh, LV_ANIM_ON);
    lv_roller_set_selected(rollerMin, mm, LV_ANIM_ON);

    lv_obj_add_event_cb(ok, event_ok, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(cancel, event_cancel, LV_EVENT_CLICKED, NULL);

}

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);
    if(code == LV_EVENT_VALUE_CHANGED && !isScrolling) {
        lv_calendar_date_t date;
        if (lv_calendar_get_pressed_date(obj, &date)){
            lv_calendar_set_today_date(obj, date.year, date.month, date.day);

        }
    }
}

static void event_ok(lv_event_t * e){

    const lv_calendar_date_t * date;
    date = lv_calendar_get_today_date(calendar);

    ttgo->rtc->setDateTime(date->year, date->month, date->day, 
          lv_roller_get_selected(rollerHour), lv_roller_get_selected(rollerMin), 0);

    lv_scr_load(mainScreen);
    lv_obj_del(myScreen);
}

static void event_cancel(lv_event_t * e){
    lv_scr_load(mainScreen);
    lv_obj_del(myScreen);
}

static void  event_scrolling(lv_event_t * e){
     lv_event_code_t code = lv_event_get_code(e);
     if (code == LV_EVENT_SCROLL_BEGIN){
         isScrolling = true;
     }
     if (code == LV_EVENT_SCROLL_END){
         isScrolling = false;
     }
}