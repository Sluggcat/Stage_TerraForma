#ifndef filehandler_h
#define filehandler_h

#include "Arduino.h"
#include "RTClib.h"  
#include <string.h>
#include <SD.h>
#include "RTClib.h"
#include "Ezo_i2c.h"

void PrintHeaders(File datafile);

void PrintToFile(RTC_PCF8523 rtc, File datafile, File recentfile, float AbsPressure, float Decibars, float Meters, float Celsius, float sal_float,
                  int nbSamples, float vbatt, Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO, 
                  float average_color_readings[12], uint8_t colorList[10]);

void SDCardDateTimeCallback(uint16_t* date, uint16_t* time);

void FileCreate(DateTime now, File datafile, File recentfile);
#endif