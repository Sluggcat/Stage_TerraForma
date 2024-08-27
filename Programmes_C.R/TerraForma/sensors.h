#ifndef SENSORS_H
#define SENSORS_H


#include <avr/dtostrf.h>     // For temperature value concatenation.
#include <Adafruit_AS7341.h> // Color Sensor
#include "TSYS01.h"          // Temp. sensor
#include "MS5837.h"          // Pressure sensor
#include "Ezo_i2c.h"         // Atlas sensors related
#include "power.h"

#define latitude 45.00    //This is your deployment latitude. It is used in the pressure to depth conversion.

// Load switches command pins
  #define PIN_EC  13
  #define PIN_DO  12
  #define PIN_ORP 11
  #define PIN_PH  10

// Salinity calculation parameters
  const float SalA1 = 2.070e-5;     const float SalA3 = 3.989e-15;
  const float SalA2 = -6.370e-10;

  const float SalB1 = 3.426e-2;    const float SalB3 = 4.215e-1;
  const float SalB2 = 4.464e-1;    const float SalB4 = -3.107e-3;
  
  const float Salc0 = 6.766097e-1;    const float Salc3 = -6.9698e-7;
  const float Salc1 = 2.00564e-2;
  const float Salc2 = 1.104259e-4;    const float Salc4 = 1.0031e-9;

  const float Sala0 = 0.0080;     const float Sala3 = 14.0941;
  const float Sala1 = -0.1692;    const float Sala4 = -7.0261;
  const float Sala2 = 25.3851;    const float Sala5 = 2.7081;

  const float Salb0 = 0.0005;     const float Salb3 = -0.0375;
  const float Salb1 = -0.0056;    const float Salb4 = 0.0636;
  const float Salb2 = -0.0066;    const float Salb5 = -0.0144;

  const float Salk = 0.0162;
  const float SalCStandard = 42.914;



// Functions
  bool yourTimeOutCheck();
  float PressureZero(MS5837 psensor);
  float AirTemperature(TSYS01 tsensor);
  void get_pressure_depth(MS5837 psensor, float* Decibars, float* Meters, float* Feet, float*Fathoms, float* AtmP, float* AbsPressure);
  void get_temperature(TSYS01 tsensor, float* Celsius, float* Fahrenheit, float* Kelvin);
  float calc_salinity(float ec_float, float Celsius, float Decibars);

  float AS7341fullScale(Adafruit_AS7341 as7341);
  void AS7341increaseGain(Adafruit_AS7341 as7341, as7341_gain_t myGAIN);
  void AS7341decreaseGain(Adafruit_AS7341 as7341, as7341_gain_t myGAIN);
  void AS7341gainControl(Adafruit_AS7341 as7341, as7341_gain_t myGAIN, uint16_t RAW_color_readings[12]);

  float receive_reading(Ezo_board &Sensor);
  void Ezo_2by2(PCA9540B pca9540b, Ezo_board EC,Ezo_board PH, Ezo_board DO, Ezo_board ORP, float Celsius, float* ec_val, float* ph_val, float* do_val, float* orp_val);
  void Ezo_4once(PCA9540B pca9540b, Ezo_board EC,Ezo_board PH, Ezo_board DO, Ezo_board ORP,float Celsius, float* ec_val, float* ph_val, float* do_val, float* orp_val);
#endif