#include "datahandler.h"

void Datalogger_setup(RTC_PCF8523 rtc) {
  // SD Did not work before without this trick, right now works fine without it (maybe it was only a Chip Select issue)
    pinMode(SD_CS, OUTPUT); 
    digitalWrite(8, HIGH);  // pin 8 is BLE Chip Select

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Check if the RTC lost power and if so, set the time
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    // The following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize the SD card
  if (!SD.begin(SD_CS)) {
    Serial.println("Initialization of SD card failed!");
    return;
  }
  Serial.println("Initialization of SD card successful.");
}

void read_RTC(RTC_PCF8523 rtc){ // try to read actual Date
  DateTime now = rtc.now(); // <== main programm CRASH when calling this function
  Serial.println("Reading RTC...");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}