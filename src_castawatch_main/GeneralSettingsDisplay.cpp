#include "config.h"
#include "GeneralSettingsDisplay.h"
#include "CastaWatchCommon.h"

static lv_obj_t * myScreen;
bool updated = false;

void LaunchGeneralSettings(){
    updated = false;

    myScreen =  lv_obj_create(NULL);
    lv_obj_set_size(myScreen, 240, 240);
    lv_scr_load(myScreen);    
    lv_obj_set_scroll_dir(myScreen, LV_DIR_VER);

    static lv_style_t lblStyle;
    lv_style_init(&lblStyle);
    lv_style_set_text_font(&lblStyle, &lv_font_montserrat_22);

    // Use 24HS
    lv_obj_t * lbl = lv_label_create(myScreen);
    lv_label_set_text(lbl, "Format 24HS");
    lv_obj_add_style(lbl, &lblStyle, LV_PART_MAIN);

    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 0,0);

    lv_obj_t * sw = lv_switch_create(myScreen);
    if (use24HS){
         lv_obj_add_state(sw, LV_STATE_CHECKED);
    }
    lv_obj_align(sw, LV_ALIGN_TOP_RIGHT,-10,0);
     lv_obj_add_event_cb(sw, event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    //Brightness
    lbl = lv_label_create(myScreen);
    lv_label_set_text(lbl, "BackLight");
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 0,50);
    lv_obj_add_style(lbl, &lblStyle, LV_PART_MAIN);

    lv_obj_t * slider = lv_slider_create(myScreen);
    lv_slider_set_range(slider, 1,255);
    lv_slider_set_mode(slider, LV_SLIDER_MODE_NORMAL);
    lv_obj_set_size(slider, 120,20);
    lv_obj_align(slider, LV_ALIGN_TOP_RIGHT, -10, 50);
    lv_slider_set_value(slider, brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, event_slider, LV_EVENT_VALUE_CHANGED, NULL);

    // Volume
    lbl = lv_label_create(myScreen);
    lv_label_set_text(lbl, "Volume");
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 0,100);
    lv_obj_add_style(lbl, &lblStyle, LV_PART_MAIN);

    slider = lv_slider_create(myScreen);
    lv_slider_set_range(slider, 0,100);
    lv_slider_set_mode(slider, LV_SLIDER_MODE_NORMAL);
    lv_obj_set_size(slider, 120,20);
    lv_obj_align(slider, LV_ALIGN_TOP_RIGHT, -10, 100);
    lv_slider_set_value(slider, (int)(volume * 100), LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, event_slider_volume, LV_EVENT_VALUE_CHANGED, NULL);



    //Close
     lv_obj_t * btn = lv_btn_create(myScreen);

    static lv_style_t btnStyle;
    lv_style_init(&btnStyle);
    lv_style_set_text_font(&btnStyle, &lv_font_montserrat_22);
    lv_style_set_radius(&btnStyle, 50);

    lv_obj_add_style(btn, &btnStyle, LV_PART_MAIN);
    lbl = lv_label_create(btn);
    lv_obj_align(lbl, LV_ALIGN_CENTER,0,0);
    lv_label_set_text(lbl, "Close");

    lv_obj_add_event_cb(btn, event_close, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0,0);
}

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        use24HS = lv_obj_has_state(obj, LV_STATE_CHECKED);
        updated = true;
    }
}

static void event_close(lv_event_t * e)
{
    lv_scr_load(mainScreen);
    lv_obj_del(myScreen);
    if (updated){
        SaveConfig();
    }
}


static void event_slider(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        brightness = lv_slider_get_value(obj);
        updated = true;
    }

}

static void event_slider_volume(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        volume = (float)lv_slider_get_value(obj) / 100.0f;
        out->SetGain(volume);
        updated = true;
    }

}