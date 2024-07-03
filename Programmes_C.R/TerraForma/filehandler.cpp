#include "filehandler.h"


void Datalogger_setup(RTC_PCF8523 rtc) {
  // trick to fix CS conflict between datalogger/BLE
    pinMode(10, OUTPUT);    // keep this
    digitalWrite(8, HIGH);  // force radio module to release MOSI

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

void PrintHeaders(File datafile) {  //Prints a header line to the CSV for column identification.
  if(datafile){
    datafile.print("Date");
    datafile.print(",");
    datafile.print("PST");
    datafile.print(",");
    datafile.print("abspres");
    datafile.print(",");
    datafile.print("dbars");
    datafile.print(",");
    datafile.print("meters");
    datafile.print(",");
    datafile.print("degC");
    datafile.print(",");

  #if USE_ATLAS
    datafile.print("EZOsal");
    datafile.print(",");
    datafile.print("PSS-78");
    datafile.print(",");
    datafile.print("EZO_EC");
    datafile.print(",");
    datafile.print("EZO_pH");
    datafile.print(",");
    datafile.print("EZO_ORP");
    datafile.print(",");
    datafile.print("EZO_DO");
    datafile.print(",");
  #endif
    datafile.print("F1_415nm");
    datafile.print(",");
    datafile.print("F2_445nm");
    datafile.print(",");
    datafile.print("F3_480nm");
    datafile.print(",");
    datafile.print("F4_515nm");
    datafile.print(",");
    datafile.print("F5_555nm");
    datafile.print(",");
    datafile.print("F6_590nm");
    datafile.print(",");
    datafile.print("F7_630nm");
    datafile.print(",");
    datafile.print("F8_680nm");
    datafile.print(",");
    datafile.print("F9_Clear");
    datafile.print(",");
    datafile.print("F10_NIR");
    datafile.print(",");
    datafile.print("NB samples");
    datafile.flush();
  }
  else{
    Serial.println("Error writing headers");
  }
}

void Datalogger_file(RTC_PCF8523 rtc, File datafile) {
  DateTime now = rtc.now();

  char filename[20];
  snprintf(filename, sizeof(filename), "%02d%02d%02d.csv", 
           now.year(), now.month(), now.day());

  if(!SD.exists(filename)){ // if file doesn't exist yet 
    Serial.print("Creating file: ");
    Serial.println(filename);
    datafile = SD.open(filename, FILE_WRITE);
    PrintHeaders(datafile); // write header
  }
  else{ // if file is already on SD
    datafile = SD.open(filename, FILE_WRITE);
    if (datafile) {
      datafile.seek(datafile.size()); // Move to the end of the file
      Serial.println("File opened for appending data.");
    } else {
      Serial.println("Error opening file for appending!");
      return;
    }
  }
}