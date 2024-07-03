#ifndef filehandler_h
#define filehandler_h

#include "Arduino.h"
#include "RTClib.h"  
#include "Ezo_i2c.h"
#include "sensors.h"
#include <string.h>
#include <SD.h>
#include <SPI.h>

#define SD_DEBUG 1 
#define SD_CS 10

void PrintHeaders(File datafile);
void Datalogger_setup(RTC_PCF8523 rtc);
void Datalogger_file(RTC_PCF8523 rtc, File datafile);

#endif