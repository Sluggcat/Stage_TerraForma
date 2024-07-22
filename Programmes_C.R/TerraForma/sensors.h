#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Adafruit_AS7341.h> // Color Sensor
#include "TSYS01.h"   // Temperature sensor
#include "MS5837.h"   // Pressure sensor
#include <avr/dtostrf.h> //For temperature value concatenation.
#include "Ezo_i2c.h"

#define latitude 45.00    //This is your deployment latitude. It is used in the pressure to depth conversion.

// Load switches command pins
  #define PIN_EC  13
  #define PIN_DO  12
  #define PIN_ORP 11
  #define PIN_PH  10

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
  void get_voltage(float* vbatt);

// Classes
/**
 *  @brief PCA9540B 2-channel I²C-bus multiplexer constructor. 
*/
class PCA9540B {
  private:
    uint8_t _address;
    const uint8_t CHANNEL_0 = 0x04;
    const uint8_t CHANNEL_1 = 0x05;

  public:
    // Constructor: Initializes the I2C address
    PCA9540B(uint8_t address = 0x70);

    // Initialize the I2C bus
    void begin();

    // Select a channel (0 or 1)
    void selectChannel(uint8_t channel);
};
  
#endif