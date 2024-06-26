#include <Arduino.h>
#include "sensors.h"

  //  Salinty calculation parameters
    const float SalA1 = 2.070e-5;
    const float SalA2 = -6.370e-10;
    const float SalA3 = 3.989e-15;

    const float SalB1 = 3.426e-2;
    const float SalB2 = 4.464e-1;
    const float SalB3 = 4.215e-1;
    const float SalB4 = -3.107e-3;

    const float Salc0 = 6.766097e-1;
    const float Salc1 = 2.00564e-2;
    const float Salc2 = 1.104259e-4;
    const float Salc3 = -6.9698e-7;
    const float Salc4 = 1.0031e-9;

    const float Sala0 = 0.0080;
    const float Sala1 = -0.1692;
    const float Sala2 = 25.3851;
    const float Sala3 = 14.0941;
    const float Sala4 = -7.0261;
    const float Sala5 = 2.7081;

    const float Salb0 = 0.0005;
    const float Salb1 = -0.0056;
    const float Salb2 = -0.0066;
    const float Salb3 = -0.0375;
    const float Salb4 = 0.0636;
    const float Salb5 = -0.0144;

    const float Salk = 0.0162;
    const float SalCStandard = 42.914;

/*----------Functions---------*/
/* @brief Calculate atmospheric pressure when the prove is above water (as it should be in normal use-case).
 * 
 * @param MS5837 psensor
 * 
 * @return AtmP (atmospheric pressure above water)
 */
float PressureZero(MS5837 psensor){  //Determines atmospheric pressure if the user turns the unit on above water.
  psensor.read();
  float AtmP = psensor.pressure();
  return AtmP;
}


/* @brief Reads air temperature when the probe is above water (as it should be in normal use-case).
 * 
 * @param TSYS01 tsensor
 * 
 * @return AirTemp (air temperature above water)
 */
float AirTemperature(TSYS01 tsensor){  //Determines air temperature if the user turns the unit on above water.
  tsensor.read();
  float AirTemp = tsensor.temperature();
  return AirTemp;
}

/* @brief IConversion from pressure to depth. See AN69 by Seabird Scientific.
 * 
 * @param MS5837 psensor, float* Decibars, float* Meters, float* Feet, float*Fathoms, float* AtmP, float* AbsPressure
 * 
 * @return none
 */
void get_pressure_depth(MS5837 psensor, float* Decibars, float* Meters, float* Feet, float*Fathoms, float* AtmP, float* AbsPressure){  
  psensor.read();  //Read the pressure sensor.
  
  *AbsPressure = psensor.pressure();
  *AbsPressure = *AbsPressure + 21; //Offset for this particular pressure sensor. Measured against baro sensor on RV Sikuliaq. Verified by CE02SHSM baro sensor.
  
  float x = sin(latitude / 57.29578);
  x = x * x;
  float gr = 9.780318 * (1.0 + (5.2788e-3 + 2.36e-5 * x) * x) + 1.092e-6 * (*Decibars);

  *Decibars = (psensor.pressure() - (*AtmP) )/100; 
  *Meters = ((((-1.82e-15 * (*Decibars) + 2.279e-10) * (*Decibars) - 2.2512e-5) * (*Decibars) + 9.72659) * (*Decibars) )/gr; //Depth in meters.
  *Feet = *Meters * 3.28084;
  *Fathoms = *Feet/6;
}

/* @brief Reads the temperature, and calculate in various units. Temperature is stored in global variables.
 * 
 * @param 
 * 
 * @return
 */
void get_temperature(TSYS01 tsensor, float* Celsius, float* Fahrenheit, float* Kelvin){ 
  tsensor.read();
  *Celsius = tsensor.temperature();
  *Fahrenheit = *Celsius*1.8+32;     
  *Kelvin = *Celsius+273.15;     
}

/* @brief Calculates salinity according to the Practical Salinity Scale (PSS-78 ).
 * 
 * @param
 * 
 * @return
 */
float calc_salinity(float ec_float, float Celsius, float Decibars)
{  //PSS-78
  float R, RpNumerator, RpDenominator, Rp, rT, RT, Salinity ;
  R = ((ec_float/1000)/SalCStandard);   
  RpNumerator = ( SalA1*Decibars)*( SalA2*pow(Decibars,2))+( SalA3*pow(Decibars,3));
  RpDenominator = 1*( SalB1*Celsius)+( SalB2*pow(Celsius,2))+( SalB3*R)+( SalB4*Celsius*R);
  Rp = 1+(RpNumerator/RpDenominator);
  rT =  Salc0 +( Salc1*Celsius)+( Salc2*pow(Celsius,2))+( Salc3*pow(Celsius,3))+( Salc4*pow(Celsius,4));
  RT=R/(rT*Rp);
  Salinity = ( Sala0+( Sala1*pow(RT,0.5))+( Sala2*RT)+( Sala3*pow(RT,1.5))+( Sala4*pow(RT,2))+( Sala5*pow(RT,2.5)))+((Celsius-15)/(1+ Salk*(Celsius-15)))*( Salb0+( Salb1*pow(RT,0.5))+( Salb2*RT)+( Salb3*pow(RT,1.5))+( Salb4*pow(RT,2))+( Salb5*pow(RT,2.5)));
  
  return Salinity;
}