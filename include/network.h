#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;


void connectWiFi(const char* ssid, const char* password);
void initNTP();
void setupWebServer();

void setRelay(bool state);
void saveMode();
void setPersistedThresholds(int onPct, int offPct);
int readAvg();
int rawToPercent(int raw, int max, int min);
void printSchedule();



#endif
