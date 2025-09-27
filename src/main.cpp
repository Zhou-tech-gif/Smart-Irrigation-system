#include <Arduino.h>
#include "irrigation.h"
#include "network.h"

// WiFi credentials
const char* ssid = "Surendra";
const char* password = "surendra123456";

void setup() {
  Serial.begin(115200);

  initIrrigation();
  connectWiFi(ssid, password);
  initNTP();
  loadMode();
  loadSchedule();
  setupWebServer();

  Serial.println("Soil moisture irrigation controller started");
}

void loop() {
  int raw = readAvg();
  int pct = rawToPercent(raw, 4095, 1500); // calibrate properly
  struct tm now = getCurrentTime();

  switch (irrigationMode) {
    case MODE_MANUAL:
      break;
    case MODE_MOISTURE:
      controlIrrigation(pct);
      checkSafety();
      break;
    case MODE_TIMER:
      checkIrrigation(now);
      checkSafety();
      break;
  }

  Serial.printf("Raw: %d, Moisture: %d%%, Relay: %s\n",
              raw, pct, relayState ? "ON" : "OFF");
  delay(1500);
}
