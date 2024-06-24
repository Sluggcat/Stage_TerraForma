#ifndef ATLAS_H
#define ATLAS_H

#include <Ezo_i2c.h> //include the EZO I2C library from https://github.com/Atlas-Scientific/Ezo_I2c_lib
#include <SD.h>  
#include <Wire.h>
#include <SoftwareSerial.h>

void init_atlas (Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO);
void mesure_atlas(File datafile, Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO);
void send_read_atlas(Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO);
void serial_value_atlas(Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO);

#endif

