#ifndef datahandler_h
#define datahandler_h

#include "Arduino.h"
#include "RTClib.h"  
#include <SD.h>
#include <SPI.h>

#define SD_DEBUG 1 
#define SD_CS 10

void Datalogger_setup(RTC_PCF8523 rtc);
void read_RTC(RTC_PCF8523 rtc);


#endif