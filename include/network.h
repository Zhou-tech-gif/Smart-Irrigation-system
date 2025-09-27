#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

extern int irrigationMode;
extern int irrigationHour;
extern int irrigationMinute;
extern int irrigationDuration;

void setRelay(bool state);
void saveMode();
void setPersistedThresholds(int onPct, int offPct);
int readAvg();
int rawToPercent(int raw, int max, int min);
void printSchedule();

// irrigation modes
#define MODE_MANUAL   0
#define MODE_MOISTURE 1
#define MODE_TIMER    2
#endif
