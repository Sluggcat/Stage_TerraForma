#ifndef power_h
#define power_h

#include "Arduino.h" 
#include <Wire.h>
#include "ArduinoLowPower.h"

#define PIN_POWER  9

void power_setup();
void power_dummy();

// Classes
/**
 *  @brief PCA9540B 2-channel I²C-bus multiplexer. 
*/
class PCA9540B {
  private:
    uint8_t _address;
    const uint8_t CHANNEL_0 = 0x04;
    const uint8_t CHANNEL_1 = 0x05;
    const uint8_t DEFAULT = 0x00;

  public:
    // Constructor: Initializes the I2C address
    PCA9540B(uint8_t address = 0x70);

    // Initialize the I2C bus
    void begin();

    // Select a channel (0 or 1)
    void selectChannel(uint8_t channel);
};
#endif