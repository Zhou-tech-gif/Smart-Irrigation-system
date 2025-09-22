#include "irrigation.h"

// --- Pin setup ---
const int moisture_pin = 34;
const int probe_pwr_pin = 25;
const int relay_pin = 4;
const bool relay_active_low = true;

// Calibration
int dryReading = 4095;
int wetReading = 1500;

// Sampling
const int n_samples = 20;
const int MEASURE_MS = 100;

// Thresholds (percent, persisted)
int water_on_percent_default  = 30;
int water_off_percent_default = 45;
int water_on_percent;
int water_off_percent;

// Timings
const unsigned long MIN_ON_MS  = 10UL * 1000UL;
const unsigned long MIN_OFF_MS = 30UL * 1000UL;
const unsigned long MAX_ON_MS  = 10UL * 60UL * 1000UL;

Preferences prefs;

// State
unsigned long lastRelayChangeTime = 0;
bool relayState = false;
int irrigationHour = 7;
int irrigationMinute = 0;
int irrigationDuration = 2 * 60 * 1000;
bool irrigatedToday = false;
unsigned long irrigationStartTime = 0;
bool irrigationRunning = false;

IrrigationMode irrigationMode = MODE_MOISTURE;

// --- Functions ---
void setRelay(bool on) {
  relayState = on;
  if (relay_active_low)
    digitalWrite(relay_pin, on ? LOW : HIGH);
  else
    digitalWrite(relay_pin, on ? HIGH : LOW);

  lastRelayChangeTime = millis();
  Serial.printf("Relay set %s at %lu\n", on ? "ON" : "OFF", lastRelayChangeTime);
}

struct tm getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    timeinfo.tm_year = 0;
  }
  return timeinfo;
}

void checkIrrigation(struct tm currentTime) {
  int currentHour = currentTime.tm_hour;
  int currentMinute = currentTime.tm_min;

  if (currentHour == irrigationHour && currentMinute == irrigationMinute && !irrigatedToday && !irrigationRunning) {
    Serial.println("Irrigation started...");
    setRelay(true);
    irrigationStartTime = millis();
    irrigationRunning = true;
    irrigatedToday = true;
  }

  if (irrigationRunning && (millis() - irrigationStartTime >= irrigationDuration)) {
    setRelay(false);
    Serial.println("Irrigation finished.");
    irrigationRunning = false;
  }

  if (currentHour == 0 && currentMinute == 0) {
    irrigatedToday = false;
  }
}

void controlIrrigation(int pct) {
  if (!relayState) {
    if (pct <= water_on_percent) {
      if (millis() - lastRelayChangeTime >= MIN_OFF_MS) setRelay(true);
      else Serial.println("ON blocked: MIN_OFF_MS not passed.");
    }
  } else {
    if (pct >= water_off_percent) {
      if (millis() - lastRelayChangeTime >= MIN_ON_MS) setRelay(false);
      else Serial.println("OFF blocked: MIN_ON_MS not passed.");
    }
  }
}

void checkSafety() {
  if (relayState && (millis() - lastRelayChangeTime >= MAX_ON_MS)) {
    Serial.println("⚠️ Safety cutoff: MAX_ON_MS reached");
    setRelay(false);
  }
}

int readAvg() {
  digitalWrite(probe_pwr_pin, HIGH);
  delay(MEASURE_MS);

  long sum = 0;
  for (int i = 0; i < n_samples; ++i) {
    sum += analogRead(moisture_pin);
    delay(2);
  }

  digitalWrite(probe_pwr_pin, LOW);
  return (int)(sum / n_samples);
}

int rawToPercent(int raw, int dryVal, int wetVal) {
  if (dryVal == wetVal) return 0;
  long mapped;
  if (dryVal > wetVal)
    mapped = map(raw, dryVal, wetVal, 0, 100);
  else
    mapped = map(raw, wetVal, dryVal, 0, 100);

  if (mapped < 0) mapped = 0;
  if (mapped > 100) mapped = 100;
  return (int)mapped;
}

void setPersistedThresholds(int onPct, int offPct) {
  if (onPct < 0) onPct = 0;
  if (offPct > 100) offPct = 100;
  water_on_percent = onPct;
  water_off_percent = offPct;

  prefs.begin("moisture", false);
  prefs.putInt("on_pct", water_on_percent);
  prefs.putInt("off_pct", water_off_percent);
  prefs.end();

  Serial.printf("Saved thresholds -> ON:%d OFF:%d\n", water_on_percent, water_off_percent);
}

void saveMode() {
  prefs.begin("moisture", false);
  prefs.putInt("mode", irrigationMode);
  prefs.end();
}

void loadMode() {
  prefs.begin("moisture", false);
  irrigationMode = (IrrigationMode)prefs.getInt("mode", MODE_MOISTURE);
  prefs.end();
}

void loadSchedule() {
  prefs.begin("schedule", false);
  irrigationHour = prefs.getInt("hour", 7);
  irrigationMinute = prefs.getInt("minute", 0);
  irrigationDuration = prefs.getInt("duration", 120000);
  prefs.end();
}

void initIrrigation() {
  pinMode(probe_pwr_pin, OUTPUT);
  digitalWrite(probe_pwr_pin, LOW);

  pinMode(relay_pin, OUTPUT);
  setRelay(false);

  analogSetPinAttenuation(moisture_pin, ADC_11db);

  prefs.begin("moisture", false);
  water_on_percent  = prefs.getInt("on_pct",  water_on_percent_default);
  water_off_percent = prefs.getInt("off_pct", water_off_percent_default);
  prefs.end();

  lastRelayChangeTime = millis();
}
