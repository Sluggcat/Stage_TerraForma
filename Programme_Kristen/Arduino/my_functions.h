#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include "TSYS01.h"
#include "MS5837.h"

float AirTemperature(TSYS01 tsensor);
float get_temperature(TSYS01 tsensor);
float PressureZero(MS5837 psensor);
float get_pressure_depth(MS5837 psensor);
float get_depth_meters(float AbsPressure, float AtmP, float latitude);

#endif

