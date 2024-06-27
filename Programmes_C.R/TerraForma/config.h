#ifndef Config_h
#define Config_h

  #include "Arduino.h" 
  #include <Wire.h>
  #include "sensors.h"
  #include "filehandler.h"

  // USER PARAMETERS : enable what you need
  #define DEBUG_SERIALPRINT   1

  #define USE_LORA            0     // If we use Adafruit Feather M0 LoRa
  #define USE_BLE             1     // If we use Adafruit Feather M0 Bluefruit
  #define USE_BLYNK           0     // Only valid with the Bluefruit.

  #define LOW_POWER_MODE      0     // TODO
  #define USE_ATLAS           1
  #define USE_OLED            1
  
  const String BROADCAST_NAME = "Econect Mk1";  //You can name your CTD anything!
  /*
  #if USE_LORA
    #include <RH_RF95.h>

    // for feather m0
    #define RFM95_CS 8
    #define RFM95_RST 4
    #define RFM95_INT 3
    // Change to 433.0 or other frequency, must match RX's freq!
    #define RF95_FREQ 869.5
    
    // Singleton instance of the radio driver
    RH_RF95 rf95(RFM95_CS, RFM95_INT);
  #endif
  */
  #if USE_BLE
    #include <Adafruit_BLE.h> 
    #include <Adafruit_BluefruitLE_SPI.h>
    #include "Adafruit_BluefruitLE_UART.h"

    #define BLUEFRUIT_SPI_CS 8
    #define BLUEFRUIT_SPI_IRQ 7
    #define BLUEFRUIT_SPI_RST 4 
    
    void CommandMode(Adafruit_BluefruitLE_SPI ble, Adafruit_AS7341 as7341, as7341_gain_t myGAIN, File datafile, File recentfile, 
                     float integrationTime, RTC_PCF8523 rtc, float AtmP, float AirTemp);

    #if USE_BLYNK
      #define BLYNK_USE_DIRECT_CONNECT
      #define BLYNK_PRINT Serial
      #include <BlynkSimpleSerialBLE.h>
      // You should get Auth Token in the Blynk App.
      // Go to the Project Settings (nut icon).
      char auth[] = "EfKrQaamWBdiUzC7vkBl8ZCwT8sQ5NqM";
    #endif
  #endif
    
  #if USE_ATLAS
    #include <Ezo_i2c.h> //include the EZO I2C library from https://github.com/Atlas-Scientific/Ezo_I2c_lib
  #endif

  #if SOFTWARE_SERIAL_AVAILABLE
    #include <SoftwareSerial.h>
  #endif

  #if USE_OLED
    #include <Wire.h>
    #include <Adafruit_GFX.h>
    
    // OLED FeatherWing buttons map to different pins depending on board:
    #if defined (ESP8266)
      #define BUTTON_A  0
      #define BUTTON_B 16
      #define BUTTON_C  2
    #elif defined(ESP32)
      #define BUTTON_A 15
      #define BUTTON_B 32
      #define BUTTON_C 14
    #elif defined(ARDUINO_STM32_FEATHER)
      #define BUTTON_A PA15
      #define BUTTON_B PC7
      #define BUTTON_C PC5
    #elif defined(TEENSYDUINO)
      #define BUTTON_A  4
      #define BUTTON_B  3
      #define BUTTON_C  8
    #elif defined(ARDUINO_NRF52832_FEATHER)
      #define BUTTON_A 31
      #define BUTTON_B 30
      #define BUTTON_C 27
    #else // 32u4, M0, M4, nrf52840 and 328p
      #define BUTTON_A  9
      #define BUTTON_B  6
      #define BUTTON_C  5
    #endif

  #endif
  
#endif