#include <Arduino.h>
#include <WiFi.h>
//#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//wifi init
#ifndef STASSID
#define STASSID "XXXXXXXXX"
#define STAPSK  "XXXXXXXX"
#endif


void initOTA();
void loopOTA();  
