#include <Arduino.h>
#include <WiFi.h>
//#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//wifi init
#ifndef STASSID
#define STASSID "PathakWiFi"
#define STAPSK  "224262822"
#endif


void initOTA();
void loopOTA();  