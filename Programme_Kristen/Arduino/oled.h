#include <Wire.h>
#include <Adafruit_GFX.h>
// Declare the OLED display  
#include <Adafruit_SH110X.h>

void init_oled(Adafruit_SH1107 oled);
void as7341_error_oled(Adafruit_SH1107 oled);
void print_colorlist_oled(Adafruit_SH1107 oled, uint8_t colorList[], uint16_t RAW_color_readings[]);

