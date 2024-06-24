#ifndef BLE_H
#define BLE_H

#include <Adafruit_BLE.h> 
#include <Adafruit_BluefruitLE_SPI.h>
#include "Adafruit_BluefruitLE_UART.h"
#include <BlynkSimpleSerialBLE.h>

void init_BLE(Adafruit_BLE ble);
void init_BLYNK(Adafruit_BLE ble);
void start_BLE(Adafruit_BLE ble, char buf[60]);
void print_color_BLE(Adafruit_BLE ble, float average_color_readings[12]);
void write_BLE(float Celsius, float AbsPressure, Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO, int USE_ATLAS);
void run_blynk_BLE();
void CommandMode();

#endif

