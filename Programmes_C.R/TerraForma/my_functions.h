#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H
using namespace std ;


#include <Adafruit_AS7341.h>
#include "TSYS01.h"
#include "MS5837.h"
#include "RTClib.h" 
#include <string.h>
#include <SD.h>  
#include <Wire.h>
#include <avr/dtostrf.h> //For temperature value concatenation.
#include <Ezo_i2c.h>

float AirTemperature(TSYS01 tsensor);
float get_temperature(TSYS01 tsensor);
float PressureZero(MS5837 psensor);
float get_pressure_depth(MS5837 psensor);
float get_depth_meters(float AbsPressure, float AtmP, float latitude);
float get_voltage();
void SDCardDateTimeCallback(uint16_t* date, uint16_t* time, RTC_PCF8523 rtc) ;
void PrintHeaders(File datafile, int USE_ATLAS);
int print_info(RTC_PCF8523 rtc, int data, File datafile, float AbsPressure, float Decibars, float Meters, float Celsius, float average_color_readings[], uint8_t nbSamples);
bool yourTimeOutCheck();
float receive_reading(Ezo_board &Sensor);
void AS7341increaseGain(Adafruit_AS7341 as7341, as7341_gain_t myGAIN);
void AS7341decreaseGain(Adafruit_AS7341 as7341, as7341_gain_t myGAIN);
float AS7341fullScale(Adafruit_AS7341 as7341);
void AS7341gainControl(Adafruit_AS7341 as7341,as7341_gain_t myGAIN, uint16_t RAW_color_readings[]);

#endif

