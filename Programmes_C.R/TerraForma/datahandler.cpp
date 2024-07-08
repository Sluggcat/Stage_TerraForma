#include "datahandler.h"

void Datalogger_setup(RTC_PCF8523 rtc) {
  // SD Did not work before without this trick, right now works fine without it (maybe it was only a Chip Select issue)
    pinMode(SD_CS, OUTPUT); 
    digitalWrite(8, HIGH);  // pin 8 is BLE Chip Select

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
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

void read_RTC(RTC_PCF8523 rtc){ //DO NOT CALL THIS IN MAIN (FREEZE SYSTEM) 
  // trying to read actual Date
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

Measure_sender::Measure_sender(int baudRate, int nb_datas) {
  this->baudRate = baudRate;
  this->nb_datas = nb_datas;
}

void Measure_sender::begin(){
  Serial1.begin(baudRate);
}

void Measure_sender::sendData(float *data){
/*
  this->around(data);

  // Create a byte array to hold the float values
  int totalSize = nb_datas * sizeof(float);
  byte byteArray[totalSize];

  // Copy the float values into the byte array
  memcpy(byteArray, data, totalSize);

  // Send the data frame
  Serial1.write(byteArray, totalSize);
  Serial1.print("\n");

  #if LOGGER_DEBUG
    Serial.print("Sent frame: ");
    Serial.write(byteArray, totalSize);
    Serial.print("`\n");
  #endif
*/
  for(int i =0 ; i<nb_datas ; i++)
  {
    Serial1.print(data[i], 2);
    Serial1.print("\n");

    #if LOGGER_DEBUG
      if (i==6) Serial.print("---\n");
      Serial.print("send:");
      Serial.println(data[i]); 
    #endif
  }
}

void Measure_sender::around(float* data){
  for (int i = 0; i < nb_datas ; i++) {
    data[i] = round(data[i] * 100.0) / 100.0; // Round to two decimal places
  }
}