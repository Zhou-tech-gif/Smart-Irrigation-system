#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

extern WebServer server;

void connectWiFi(const char* ssid, const char* password);
void initNTP();
void setupWebServer();

// irrigation modes
#define MODE_MANUAL   0
#define MODE_MOISTURE 1
#define MODE_TIMER    2
#endif
