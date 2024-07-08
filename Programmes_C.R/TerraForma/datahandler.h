#ifndef datahandler_h
#define datahandler_h

#include <Arduino.h>
#include <RTClib.h>
#include <SD.h>
#include <SPI.h>

#define LOGGER_DEBUG 1
#define SD_CS 10

void Datalogger_setup(RTC_PCF8523 rtc);
void read_RTC(RTC_PCF8523 rtc);

class Measure_sender{
  public:
    Measure_sender(int baudRate, int nb_datas);
    void begin();
    void sendData(float* data);


  private:
    int baudRate, nb_datas;
    void around(float* data);
};

#endif