#include <Arduino.h>
#include <WiFi.h>
//#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//wifi init
#ifndef STASSID
#define STASSID "xxxxxxxxx"
#define STAPSK  "xxxxxxxxxx"
#endif


void initOTA();
void loopOTA();  
