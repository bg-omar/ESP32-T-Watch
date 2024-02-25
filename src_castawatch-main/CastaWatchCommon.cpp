#include "config.h"
#include "AudioFileSourcePROGMEM.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include <driver/i2s.h>
#include "CastaWatchCommon.h"
#include <Preferences.h>
#include "Alarms.h"

#include "Sounds/wara.h"
#include "Sounds/alarm1.h"
#include "Sounds/alarm2.h"
#include "Sounds/alarm3.h"
#include "Sounds/alarm4.h"

TTGOClass *ttgo;
lv_obj_t * mainScreen;
bool use24HS;
uint8_t brightness = 255;
float volume = 0.5f;
bool IsAudioOutMode = true;
uint64_t TimeToSleep = 1000 * 60 * 15 ; //15 min
uint64_t TimeToBacklightOff = 1000 * 10; // 10 sec
bool lowbright = false;
unsigned long lastTouch = 0;

AudioGeneratorMP3 *mp3;
AudioFileSourcePROGMEM *file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;

Preferences preferences;

void GoSleep(){
    Serial.println("Going to sleep");

    ttgo->displaySleep();
    ttgo->power->setPowerOutPut(AXP202_LDO3, false);
    ttgo->power->setPowerOutPut(AXP202_LDO4, false);
    ttgo->power->setPowerOutPut(AXP202_LDO2, false);
    // The following power channels are not used
    ttgo->power->setPowerOutPut(AXP202_EXTEN, false);
    ttgo->power->setPowerOutPut(AXP202_DCDC2, false);

    // PEK Button WakeUp    
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);

    // Timer WakeUP (Next configured Alarm)

    uint32_t _TimeToNextAlarm = TimeToNextAlarm() -3;
    if (_TimeToNextAlarm>0){
        uint64_t totaltime = (uint64_t)1000000 * (uint64_t)_TimeToNextAlarm;
        esp_sleep_enable_timer_wakeup(totaltime); //sec
        Serial.printf("Timer to wakeup %" PRIu64 "\n", totaltime);
    }
  
    esp_deep_sleep_start();


}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    case ESP_SLEEP_WAKEUP_GPIO: Serial.println("Wakeup caused by GPIO"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void PlaySound(int index){

    if (mp3->isRunning())
    {
        mp3->stop();
    }

    // workaround to allow use I2S_0 shared between mic and speaker
    InitializeSpeaker();

    switch (index)
    {
    case 1:
        file = new AudioFileSourcePROGMEM(alarm1_sound, sizeof(alarm1_sound));
        break;
    case 2:
        file = new AudioFileSourcePROGMEM(alarm2_sound, sizeof(alarm2_sound));
        break;
    case 3:
        file = new AudioFileSourcePROGMEM(alarm3_sound, sizeof(alarm3_sound));
        break;
    case 4:
        file = new AudioFileSourcePROGMEM(alarm4_sound, sizeof(alarm4_sound));
        break;
    
    default:
        file = new AudioFileSourcePROGMEM(wara_sound, sizeof(wara_sound));
        break;
    }

    id3 = new AudioFileSourceID3(file);
    mp3->begin(id3, out);
    out->SetGain(volume);
}

void InitializeMic(){

    i2s_driver_uninstall(I2S_NUM_0);

    IsAudioOutMode = false;

     /*****************
     * mic
     * 
     * ****************/

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate =  44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_I2S,//(i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = 128,
    };


    i2s_pin_config_t i2s_cfg;
    i2s_cfg.bck_io_num   = I2S_PIN_NO_CHANGE;
    i2s_cfg.ws_io_num    = MIC_CLOCK;
    i2s_cfg.data_out_num = I2S_PIN_NO_CHANGE;
    i2s_cfg.data_in_num  = MIC_DATA;
    i2s_cfg.mck_io_num = I2S_PIN_NO_CHANGE;

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &i2s_cfg);
    i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void InitializeSpeaker(){
      /****************************
     * Initialize Audio
     * **************************/
    IsAudioOutMode = true;
    i2s_driver_uninstall(I2S_NUM_0);

    ttgo->enableAudio();
    ttgo->enableLDO3();
    #if defined(STANDARD_BACKPLANE)
        out = new AudioOutputI2S(0, 1);
    #elif defined(EXTERNAL_DAC_BACKPLANE)
        out = new AudioOutputI2S();
        //External DAC decoding
        out->SetPinout(TWATCH_DAC_IIS_BCK, TWATCH_DAC_IIS_WS, TWATCH_DAC_IIS_DOUT);
    #endif
    mp3 = new AudioGeneratorMP3();

}

void SaveConfig(){
    preferences.begin("castawatch", false);

    preferences.putBool("Use24HS", use24HS);
    preferences.putUChar("Brightness", brightness);
    preferences.putFloat("Volume", volume);

    preferences.end();
}

void LoadConfig(){

    preferences.begin("castawatch", false);

    use24HS = preferences.getBool("Use24HS", false);
    brightness = preferences.getUChar("Brightness", 255);
    volume = preferences.getFloat("Volume", 0.5f);

    preferences.end();
}

void activateScreen(){
        lastTouch = millis();
        lowbright = false;
}
