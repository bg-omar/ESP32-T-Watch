#include "CastaWatchCommon.h"
#include <Preferences.h>
#include "Alarms.h"
#include <UnixTime.h>

Preferences prefs;

uint8_t AlarmCount;
schedule_t Alarms[10]; // max alarms
int AlarmScheduled = -1; 

void InitializeAlarms(){
    prefs.begin("alarms",false);
    AlarmCount = prefs.getChar("AlarmCount",0);
    Serial.printf("Alarms: %d \n",AlarmCount);
    size_t length = prefs.getBytesLength("AlarmList");
//    Serial.printf("AlarmList Length: %d \n",length);
    size_t result = prefs.getBytes("AlarmList",Alarms, length);
//    Serial.printf("AlarmList Bytes: %d \n",result);
    prefs.end();
}

void DeleteAlarm(int index){
    for(int i=index;i<9;i++){
        Alarms[i] = Alarms[i+1];
    }
    AlarmCount--;
    SaveAlarmConfiguration();
}

void AddNewAlarm(schedule_t newAlarm){
    AlarmCount++;
    Alarms[AlarmCount-1] = newAlarm;
    SaveAlarmConfiguration();
}

void SaveAlarmConfiguration(){

    prefs.begin("alarms",false);
    prefs.putChar("AlarmCount",AlarmCount);
    prefs.putBytes("AlarmList", &Alarms, 10 * sizeof(schedule_t));
    prefs.end();
    SetNextAlarm();

}



void SetNextAlarm(){

    UnixTime stamp(0);
    int alarmIndex = -1;
    bool hasAlarm = GetNextAlarm(stamp, alarmIndex);
    if (hasAlarm){
       ttgo->rtc->setAlarm(stamp.hour, stamp.minute, PCF8563_NO_ALARM, PCF8563_NO_ALARM);
       ttgo->rtc->enableAlarm();
       Serial.printf("Alarm Enabled %02d:%02d \n", stamp.hour, stamp.minute);
    } else {
        Serial.printf("Alarm is disabled\n");
        ttgo->rtc->disableAlarm();
    }
    AlarmScheduled = alarmIndex;

}

uint32_t TimeToNextAlarm(){
    RTC_Date now = ttgo->rtc->getDateTime();
    UnixTime stamp1(0);
    stamp1.setDateTime(now.year, now.month, now.day, now.hour,now.minute,now.second); 
    uint32_t epoch = stamp1.getUnix();
 
    UnixTime stamp(0);
    int index;
    bool success = GetNextAlarm(stamp, index);
    if (!success){
        return -1;
    }

    return stamp.getUnix() - epoch;
}

bool GetNextAlarm(UnixTime &stamp, int &alarmIndex){

    if (AlarmCount==0) return false;
    RTC_Date now = ttgo->rtc->getDateTime();
    stamp.setDateTime(now.year, now.month, now.day, now.hour,now.minute,now.second); 
    uint32_t epoch = stamp.getUnix();
    uint32_t distancemin = 0xFFFFFFFF;
//    uint8_t indexmin = 0;
    uint32_t nextalarm = 0;
    for(int i=0;i<AlarmCount;i++){
        UnixTime alarmt(0);
        alarmt.setDateTime(now.year, now.month, now.day, 
            Alarms[i].hour,Alarms[i].minute,00); 
        uint32_t tim = alarmt.getUnix();
        if (tim<=epoch) tim+=(3600 * 24);
        if ((tim-epoch)<distancemin){
            distancemin = tim-epoch;
            alarmIndex=i;
            nextalarm = tim;
        }
    }
    if (alarmIndex>=0){
        stamp.getDateTime(nextalarm);
        return true;
    } else {
        return false;
    }


}

static lv_obj_t * msg;
void event_msg(lv_event_t * e){
    if (mp3->isRunning()){
        mp3->stop();
    }
    lv_msgbox_close(msg);
}

void PlayAlarm() {
    if (AlarmScheduled<0) {
        Serial.printf("Error, No alarm to play!");
        return;
    }

    PlaySound(Alarms[AlarmScheduled].setting1);

    if (Alarms[AlarmScheduled].setting2 == 0){
        // no repeat alarm;
        DeleteAlarm(AlarmScheduled);
        AlarmScheduled = -1;
    }

    static const char * btns[] = {"OK", "", ""};

    msg = lv_msgbox_create(NULL, "Alarm!", "", btns, false);
    lv_obj_add_event_cb(msg, event_msg, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(msg);

}