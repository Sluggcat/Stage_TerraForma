#include "datahandler.h"
#include  "config.h"

// Class methods
/**
 * @brief Class constructor.
 * @param baudRate : baudRate for serial comms
 * @param nb_datas : number of datas to send.
 */
Measure_sender::Measure_sender(int baudRate, int nb_datas) {
  this->baudRate = baudRate;
  this->nb_datas = nb_datas;
}

/**
 * @brief Initialize a serial comm. on the Arduino
 */
void Measure_sender::begin(){
  Serial1.begin(baudRate);
}

/**
 * @brief Send the datas
 * @param data : pointer to an array containing the datas to send
 */
void Measure_sender::sendData(float *data){
  for(int i =0 ; i<nb_datas ; i++)
  { 
    Serial1.print(data[i], 2);
    Serial1.print("\n");

  #if DEBUG_SERIALPRINT
    #if LOGGER_DEBUG
      if (i==6) Serial.print(F("---\n"));
      Serial.print(F("send:\t"));
      Serial.println(data[i]); 
      if (i==13) Serial.print(F("===\n===\n"));
    #endif
  #endif
  }
}

// Functions
/**
  * @brief Initialize RTC and SD card
  * @param rtc 
  * @remark Spi adresses shennanigans on the datalogger: SD_CS may conflict with radio module (ex: LoRA)
  */
void Datalogger_setup(RTC_PCF8523 rtc) {
  // SD Did not work before without this trick, right now works fine without it (maybe it was only a Chip Select issue)
    pinMode(SD_CS, OUTPUT); 
    digitalWrite(8, HIGH);  // pin 8 is BLE Chip Select

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
  }

  // Check if the RTC lost power and if so, set the time
  if (rtc.lostPower()) {
    Serial.println(F("RTC lost power, setting the time!"));
    // The following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize the SD card
  if (!SD.begin(SD_CS)) {
    Serial.println(F("Initialization of SD card failed!"));
    return;
  }
  Serial.println(F("Initialization of SD card successful."));
}

/**
  * @brief Read actual Date from RTC and display it into serial monitor
  * @param rtc
  * @remark DO NOT CALL THIS IN THE PROGRAM IT CAUSES A FREEZE (maybe memory issue ? dunno) 
  */
void read_RTC(RTC_PCF8523 rtc){ //
  DateTime now = rtc.now(); // <== CRASH when calling this function
  Serial.println(F("Reading RTC..."));
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