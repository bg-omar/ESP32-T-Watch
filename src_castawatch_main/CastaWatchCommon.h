#include "config.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#ifndef CASTA_WATCH_COMMON
#define CASTA_WATCH_COMMON

#define MSG_BRIGHTNESS_CHANGED


extern TTGOClass *ttgo;
extern lv_obj_t * mainScreen;
extern bool use24HS;
extern uint8_t brightness;
extern AudioGeneratorMP3 *mp3;
extern AudioOutputI2S *out;
extern float volume;
extern uint64_t TimeToSleep; 
extern uint64_t TimeToBacklightOff;

extern bool IsAudioOutMode;
extern bool lowbright;
extern unsigned long lastTouch;

void GoSleep();
void print_wakeup_reason();
void PlaySound(int index);

void InitializeSpeaker();
void InitializeMic();

void SaveConfig();
void LoadConfig();

void activateScreen();

#endif