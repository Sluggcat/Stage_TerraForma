#ifndef SENSORS_H
#define SENSORS_H
  #include <Arduino.h>

  #include <Adafruit_AS7341.h> // Color Sensor
  #include "TSYS01.h"   // Temperature sensor
  #include "MS5837.h"   // Pressure sensor
  #include <avr/dtostrf.h> //For temperature value concatenation.

  #define latitude 45.00    //This is your deployment latitude. It is used in the pressure to depth conversion.

  // Functions
  float PressureZero(MS5837 psensor);
  float AirTemperature(TSYS01 tsensor);
  void get_pressure_depth(MS5837 psensor, float* Decibars, float* Meters, float* Feet, float*Fathoms, float* AtmP, float* AbsPressure);
  void get_temperature(TSYS01 tsensor, float* Celsius, float* Fahrenheit, float* Kelvin);
  float calc_salinity(float ec_float, float Celsius, float Decibars);
#endif