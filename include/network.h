#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

extern WebServer server;

void connectWiFi(const char* ssid, const char* password);
void initNTP();
void setupWebServer();

#endif
