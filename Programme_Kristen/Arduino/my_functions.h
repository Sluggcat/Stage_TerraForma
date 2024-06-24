#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <Adafruit_AS7341.h>
#include "TSYS01.h"
#include "MS5837.h"
#include "RTClib.h" 
#include <string.h>
#include <SD.h>  
#include <Wire.h>
#include <avr/dtostrf.h> //For temperature value concatenation.

int init_AS7341 (Adafruit_AS7341 as7341);
float AirTemperature(TSYS01 tsensor);
float get_temperature(TSYS01 tsensor);
float PressureZero(MS5837 psensor);
float get_pressure_depth(MS5837 psensor);
float get_depth_meters(float AbsPressure, float AtmP, float latitude);
float get_voltage();
void SDCardDateTimeCallback(uint16_t* date, uint16_t* time);
void PrintHeaders(File datafile, bool USE_ATLAS);
int print_info(int data, File datafile, float AbsPressure, float Decibars, float Meters, float Celsius, float average_color_readings[], uint8_t nbSamples);
bool yourTimeOutCheck();
float receive_reading(Ezo_board &Sensor);
void AS7341increaseGain(as7341_gain_t myGAIN);
void AS7341decreaseGain(as7341_gain_t myGAIN);
float AS7341fullScale(Adafruit_AS7341 as7341);
void AS7341gainControl(uint16_t RAW_color_readings[]);

#endif

