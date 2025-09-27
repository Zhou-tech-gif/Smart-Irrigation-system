#include "network.h"
#include "irrigation.h"
#include <SPIFFS.h>
#include <time.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

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

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");

  server.on("/4/on", HTTP_GET, [](AsyncWebServerRequest *request){
    setRelay(true);
    request->send(200, "text/plain", "Relay ON");
  });

  server.on("/4/off", HTTP_GET, [](AsyncWebServerRequest *request){
    setRelay(false);
    request->send(200, "text/plain", "Relay OFF");
  });

  server.on("/mode/manual", HTTP_GET, [](AsyncWebServerRequest *request){
    irrigationMode = MODE_MANUAL;
    saveMode();
    request->send(200, "text/plain", "Mode = MANUAL");
  });

  server.on("/mode/moisture", HTTP_GET, [](AsyncWebServerRequest *request){
    irrigationMode = MODE_MOISTURE;
    saveMode();
    request->send(200, "text/plain", "Mode = MOISTURE");
  });

  server.on("/mode/timer", HTTP_GET, [](AsyncWebServerRequest *request){
    irrigationMode = MODE_TIMER;
    saveMode();
    request->send(200, "text/plain", "Mode = TIMER");
  });

  // Schedule update
  server.on("/setSchedule", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("hour", true) && request->hasParam("minute", true) && request->hasParam("duration", true)) {
      irrigationHour = request->getParam("hour", true)->value().toInt();
      irrigationMinute = request->getParam("minute", true)->value().toInt();
      irrigationDuration = request->getParam("duration", true)->value().toInt() * 1000;

      Preferences prefs;
      prefs.begin("schedule", false);
      prefs.putInt("hour", irrigationHour);
      prefs.putInt("minute", irrigationMinute);
      prefs.putInt("duration", irrigationDuration);
      printSchedule();
      prefs.end();

      request->send(200, "text/plain", "Schedule updated!");
    } else {
      request->send(400, "text/plain", "Missing parameters");
    }
  });

  // Threshold update
  server.on("/setThresholds", HTTP_POST, [](AsyncWebServerRequest *request){
    int onPct = request->getParam("on", true)->value().toInt();
    int offPct = request->getParam("off", true)->value().toInt();
    setPersistedThresholds(onPct, offPct);
    request->send(200, "text/plain", "Thresholds updated");
  });

  // Moisture data
  server.on("/moisture", HTTP_GET, [](AsyncWebServerRequest *request){
    int raw = readAvg();
    int pct = rawToPercent(raw, 4095, 1500);
    String json = "{\"moisture\":" + String(pct) + "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}