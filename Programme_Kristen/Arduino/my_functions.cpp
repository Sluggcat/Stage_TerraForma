#include "my_functions.h"


/*
 * @brief Reads air temperature when the probe is above water (as it should be in normal use-case).
 * 
 * @param
 * 
 * @return Temeprature value in float format.
 */
float AirTemperature(TSYS01 tsensor)
{  //Determines air temperature if the user turns the unit on above water.
  tsensor.read();
  return tsensor.temperature();
}

/*
 * @brief Reads the temperature, and calculate in various units. Temperature is stored in global variables.
 * 
 * @param 
 * 
 * @return
 */
float get_temperature(TSYS01 tsensor)
{ 
  tsensor.read();
  return tsensor.temperature();
  /*Fahrenheit = Celsius*1.8+32;     
  Kelvin = Celsius+273.15; */    
}

/*
 * @brief Calculate atmospheric pressure when the prove is above water (as it should be in normal use-case).
 * 
 * @param
 * 
 * @return
 */
float PressureZero(MS5837 psensor){  //Determines atmospheric pressure if the user turns the unit on above water.
  psensor.read();
  return psensor.pressure();
}

/*
 * @brief IConversion from pressure to depth. See AN69 by Seabird Scientific.
 * 
 * @param None
 * 
 * @return
 */
float get_pressure_depth(MS5837 psensor)
{  
  psensor.read();  //Read the pressure sensor.
  //A modifier pour simplifier
  AbsPressure = psensor.pressure();
  return (AbsPressure + 21); //Offset for this particular pressure sensor. Measured against baro sensor on RV Sikuliaq. Verified by CE02SHSM baro sensor.
  /*
  Decibars = (psensor.pressure()- AtmP)/100;   
  x = sin(latitude / 57.29578);
  x = x * x;
  gr = 9.780318 * (1.0 + (5.2788e-3 + 2.36e-5 * x) * x) + 1.092e-6 * Decibars;
  Meters = ((((-1.82e-15 * Decibars + 2.279e-10) * Decibars - 2.2512e-5) * Decibars + 9.72659) * Decibars)/gr; //Depth in meters.
  Feet = Meters * 3.28084;
  Fathoms = Feet/6;
  */
}

float get_depth_meters(float AbsPressure, float AtmP, float latitude)
{
    float decibars = ((AbsPressure - 21) - AtmP) / 100;
    x = sin(latitude / 57.29578);
    x = x * x;
    gr = 9.780318 * (1.0 + (5.2788e-3 + 2.36e-5 * x) * x) + 1.092e-6 * decibars;
    Meters = ((((-1.82e-15 * decibars + 2.279e-10) * decibars - 2.2512e-5) * decibars + 9.72659) * decibars)/gr; //Depth in meters.
}

