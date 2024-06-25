#include "oled.h"

#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14

void init_oled(Adafruit_SH1107 oled)
{
    oled = Adafruit_SH1107(64, 128, &Wire);
    oled.begin(0x3C, true);
    //oled.setBatteryVisible(true);
    oled.display();
    delay(1000);

    // Clear the buffer.
    oled.clearDisplay();
    oled.display();
    oled.setRotation(1);

    // text display tests
    oled.setTextSize(1);
    oled.setTextColor(SH110X_WHITE);
    oled.setCursor(0,0);

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_C, INPUT_PULLUP);
}

void as7341_error_oled(Adafruit_SH1107 oled)
{
    oled.clearDisplay();
    oled.setCursor(0,0);
    oled.println("Could not find AS7341");
    oled.display();
}

void print_colorlist_oled(Adafruit_SH1107 oled, uint8_t colorList[], uint16_t RAW_color_readings[])
{
    oled.clearDisplay();
    oled.setCursor(0,0);
    
    for(int k=0;k<10;k++){
      oled.print("F");oled.print(colorList[k]+1);oled.print(" ");
      oled.println(RAW_color_readings[colorList[k]]);
    }
    oled.display();
}

