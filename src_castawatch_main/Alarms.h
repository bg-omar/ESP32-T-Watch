#include "config.h"
#include "UnixTime.h"

#ifndef ALARMS_H
#define ALARMS_H

typedef struct {
  uint8_t hour;
  uint8_t minute;
  uint8_t setting1;
  uint8_t setting2;
} schedule_t;

extern uint8_t AlarmCount;
extern schedule_t Alarms[10];
extern int AlarmScheduled;

void InitializeAlarms();
void DeleteAlarm(int index);
void SaveAlarmConfiguration();
void AddNewAlarm(schedule_t newAlarm);
void SetNextAlarm();
uint32_t TimeToNextAlarm();
bool GetNextAlarm(UnixTime &stamp, int &alarmIndex);
void PlayAlarm();

#endif