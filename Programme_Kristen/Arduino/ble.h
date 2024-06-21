#include <Adafruit_BLE.h> 
#include <Adafruit_BluefruitLE_SPI.h>
#include "Adafruit_BluefruitLE_UART.h"
#include <BlynkSimpleSerialBLE.h>

void init_BLE();
void init_BLYNK();
void start_BLE(char buf[60]);
void print_color_BLE(float average_color_readings[12]);
void write_BLE(float Celsius, float AbsPressure, Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO, int USE_ATLAS);
void run_blynk_BLE();
void CommandMode();

