#include <Arduino.h>
#include <WiFi.h>
//#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//wifi init
#ifndef STASSID
#define STASSID "WIFI_123456789"
#define STAPSK  "FLATRONE2041"
#endif


void initOTA();
void loopOTA();  
