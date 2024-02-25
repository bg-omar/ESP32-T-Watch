#include "config.h"
#include "Functions.h"
#include "MainDisplay.h"
#include "CastaWatchCommon.h"
#include "AppList.h"
#include <driver/i2s.h>
#include "Vumeter.h"
#include "Alarms.h"

bool irq = false;
bool rtcIrq = false;

void setup() {
    Serial.begin(115200);

    print_wakeup_reason();

    log_d("Initialize Watch");
    /**************************
        Initialize Watch
     **************************/
    ttgo = TTGOClass::getWatch();

    ttgo->motor_begin();
    ttgo->motor->onec(50);

    ttgo->begin();
    ttgo->openBL();
    ttgo->setBrightness(128);
    ttgo->lvgl_begin();

    LoadConfig();

    initializeStyle();
    mainScreen = lv_scr_act();

    /***********************************
     *  Initialize Main Navigation Panes
     * *********************************/

    lv_obj_t * panel = lv_obj_create(mainScreen);
    lv_obj_set_size(panel, 240, 240);
    lv_obj_set_scroll_snap_x(panel, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scroll_snap_y(panel, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_scroll_dir(panel, LV_DIR_HOR);
    lv_obj_set_style_pad_gap(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 0,0);

    static lv_style_t scrollbarstyle;
    lv_style_init(&scrollbarstyle);
    lv_style_set_width(&scrollbarstyle,0);
    lv_style_set_height(&scrollbarstyle,0);
    lv_obj_add_style(panel, &scrollbarstyle, LV_PART_SCROLLBAR);

    /***************************
     * Main Clock Page
     * ************************/
    lv_obj_t * t1 = lv_obj_create(panel);
    lv_obj_set_size(t1, 240, 240);
    lv_obj_align(t1, LV_ALIGN_CENTER,0,0);
    lv_obj_set_style_pad_gap(t1, 0, 0);
    lv_obj_set_style_pad_all(t1, 0,0);
    initializeClock(t1);

    /*************************
     * App list 
     * **********************/
    lv_obj_t * t2 = lv_obj_create(panel);
    lv_obj_set_size(t2, 240, 240);
    lv_obj_align(t2, LV_ALIGN_CENTER,0,0);
    lv_obj_set_style_pad_gap(t2, 0, 0);
    lv_obj_set_style_pad_all(t2, 0,0);
    lv_obj_set_scroll_dir(t2, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(t2, LV_SCROLLBAR_MODE_OFF);

    initializeAppList(t2);

    /**********************
     * Force Snap on startup
     * *******************/
    lv_obj_update_snap(panel, LV_ANIM_ON);

    /*********************
     * Setup Interruptions for WakeUp
     * ******************/

    log_d("Setup Interruptions for WakeUp");
    pinMode(AXP202_INT, INPUT_PULLUP);
        attachInterrupt(AXP202_INT, [] {
            irq = true;
        }, FALLING);

    ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ,true);
    ttgo->power->clearIRQ();

    InitializeSpeaker();

    /****************************
     * Setup interruptions for alarms
     * *************************/
    log_d("Setup Interruptions for alarms");
    pinMode(RTC_INT_PIN, INPUT_PULLUP);
    attachInterrupt(RTC_INT_PIN, [] {
        rtcIrq = 1;
    }, FALLING);

    /*****************************
     * Initialize alarms
     * ***************************/
    InitializeAlarms();
    SetNextAlarm();

    Serial.println("Initialization completed");
}

unsigned long lastUpdateClock = 0;

void loop() {
    lv_task_handler();
    if (millis()-lastUpdateClock>1000){
        lastUpdateClock = millis();
        updateDisplay();
       // uint32_t heapsize = xPortGetFreeHeapSize();
       // Serial.printf("%d\n", heapsize);
    }

    int16_t x, y;
    if (ttgo->getTouch(x,y)){
        activateScreen();
    }
    if (millis()-lastTouch>TimeToBacklightOff && IsAudioOutMode){
        lowbright = true;
    }

    if (millis()-lastTouch>(TimeToSleep)){
        GoSleep();
    }

    if (irq){
        ttgo->power->readIRQ();
        if (ttgo->power->isPEKShortPressIRQ()){
            irq=false;
            ttgo->power->clearIRQ();
            GoSleep();
        }
    }
    if (lowbright){
        ttgo->setBrightness(1);
    } else {
        ttgo->setBrightness(brightness);
    }

    if (IsAudioOutMode && mp3->isRunning()){
        if (!mp3->loop()){
            mp3->stop();
        } 
    }
    if (!IsAudioOutMode)
        UpdateVumeter();

    if (rtcIrq) {
        rtcIrq = 0;
        Serial.println("Alarm!!!\n");
        ttgo->rtc->resetAlarm();
        activateScreen();
        PlayAlarm();
        SetNextAlarm();
    }
}


void initializeStyle(){

    /***************************************
     *   Set DARK Theme
     * ************************************/
    lv_theme_t * th = lv_theme_default_init( lv_disp_get_default(),
              lv_palette_darken(LV_PALETTE_BLUE, 4),
              lv_palette_main(LV_PALETTE_GREEN), true, &lv_font_montserrat_22);
    
    lv_disp_set_theme(lv_disp_get_default(), th);
    lv_disp_set_bg_color(lv_disp_get_default(), lv_color_black());
    
}

