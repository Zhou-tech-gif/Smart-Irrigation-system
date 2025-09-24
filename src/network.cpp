#include "network.h"
#include "irrigation.h"
#include <SPIFFS.h>
#include <time.h>

WebServer server(80);

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;
int raw = readAvg();
int pct = rawToPercent(raw,4095,1500);

void connectWiFi(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void initNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time synchronized");
}

void setupWebServer() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed!");
    return;
  }

  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");

  server.on("/4/on", []() {
    setRelay(true);
    server.send(200, "text/plain", "Relay ON");
  });

  server.on("/4/off", []() {
    setRelay(false);
    server.send(200, "text/plain", "Relay OFF");
  });

  server.on("/mode/manual", []() {
    irrigationMode = MODE_MANUAL;
    saveMode();
    server.send(200, "text/plain", "Mode = MANUAL");
  });

  server.on("/mode/moisture", []() {
    irrigationMode = MODE_MOISTURE;
    saveMode();
    server.send(200, "text/plain", "Mode = MOISTURE");
  });

  server.on("/mode/timer", []() {
    irrigationMode = MODE_TIMER;
    saveMode();
    server.send(200, "text/plain", "Mode = TIMER");
  });

  server.on("/setSchedule", HTTP_POST, []() {
    if (server.hasArg("hour") && server.hasArg("minute") && server.hasArg("duration")) {
      irrigationHour = server.arg("hour").toInt();
      irrigationMinute = server.arg("minute").toInt();
      irrigationDuration = server.arg("duration").toInt() * 1000;

      Preferences prefs;
      prefs.begin("schedule", false);
      prefs.putInt("hour", irrigationHour);
      prefs.putInt("minute", irrigationMinute);
      prefs.putInt("duration", irrigationDuration);
      printSchedule();
      prefs.end();


      server.send(200, "text/plain", "Schedule updated!");
    } else {
      server.send(400, "text/plain", "Missing parameters");
    }
  });
  
  server.on("/setThresholds", HTTP_POST, []() {
    int onPct = server.arg("on").toInt();
    int offPct = server.arg("off").toInt();
    setPersistedThresholds(onPct, offPct);
    server.send(200, "text/plain", "Thresholds updated");
}); 
    server.on("/moisture", []() {
    String json = "{\"moisture\":" + String(pct) + "}";
    server.send(200, "application/json", json);
  });
  



  server.begin();
}
