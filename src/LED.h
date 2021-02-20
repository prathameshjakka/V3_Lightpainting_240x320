#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP32_OTA.h>
#include <ESP32_SD_Update.h>

#include <lvgl.h>
#include <TFT_eSPI.h>
#include "FS.h"
#include "SPI.h"
#include "SD.h"
#include "PCF8574.h"
#include "FastLED.h" 

File bmpFile;
File root;
String m_CurrentFilename = "";
String m_CurrentFoldername = "";
int m_FileIndex = 0;
int m_FolderIndex = 0;
int m_NumberOfFiles = 0;
int m_NumberOfFolder = 0;
String m_FileNames[300];
String m_FolderName[300];