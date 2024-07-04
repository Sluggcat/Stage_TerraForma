/*  
  Datalogger : write files to the SD card without metadatas
      MPU Adafruit Bluefruit BLE
      Adalogger FeatherWing
    by Sluggcat
*/

#include <RTClib.h>
#include <SPI.h>
#include <SD.h> // this lib assumes Arduino has no clock so the files created have wrong dates

#define SD_DEBUG 1 

RTC_PCF8523 rtc;
File dataFile;

const int chipSelect = 10; // Change to your SD card CS pin
void PrintHeaders(File datafile);

void setup() {
  // trick to fix CS conflict between datalogger/BLE : useful first time but not required anymore !?
    //pinMode(10, OUTPUT);   
    //digitalWrite(8, HIGH);  

  Serial.begin(9600);
  while(!Serial);

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
  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization of SD card failed!");
    return;
  }
  Serial.println("Initialization of SD card successful.");
}

void loop() {
  #if SD_DEBUG
    if(SD.begin(chipSelect)){
      Serial.println("SD OK");
    }
    else{
      Serial.println("SD FAIL");
      delay(20000);
    }
  #endif

  DateTime now = rtc.now();
  char filename[20];
  snprintf(filename, sizeof(filename), "%02d%02d%02d.csv", 
           now.year(), now.month(), now.day());

  if(!SD.exists(filename)){ // if file doesn't exist yet 
    Serial.print("Creating file: ");
    Serial.println(filename);
    dataFile = SD.open(filename, FILE_WRITE);
    PrintHeaders(dataFile); // write header
  }
  else{ // if file is already on SD
    Serial.print("File found: ");
    Serial.println(filename);
    dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
      dataFile.seek(dataFile.size()); // Move to the end of the file
      Serial.println("File opened for appending data.");
    } else {
      Serial.println("Error opening file for appending!");
      return;
    }
  }
  // Write actual datas here
  Serial.println("===");
  delay(5000); // Log data every 5 seconds
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
    datafile.print("ec");
    datafile.print(",");
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