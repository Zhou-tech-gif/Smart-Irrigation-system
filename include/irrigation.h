#ifndef IRRIGATION_H
#define IRRIGATION_H

#include <Arduino.h>
#include <Preferences.h>
#include <time.h>

// --- Enums ---
enum IrrigationMode {
  MODE_MANUAL,
  MODE_MOISTURE,
  MODE_TIMER
};
extern IrrigationMode irrigationMode;

// --- Globals (accessible in main) ---
extern bool relayState;
extern int irrigationHour, irrigationMinute, irrigationDuration;

// --- Functions ---
const char* modeName(IrrigationMode mode);
void setRelay(bool on);
void controlIrrigation(int pct);
void checkIrrigation(struct tm currentTime);
void checkSafety();
int readAvg();
int rawToPercent(int raw, int dryVal, int wetVal);
struct tm getCurrentTime();
void printSchedule();
void setPersistedThresholds(int onPct, int offPct);
void saveMode();
void loadMode();
void loadSchedule();
void initIrrigation();

#endif

