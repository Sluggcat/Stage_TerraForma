#include "power.h"

/**
 * @brief Setup function to wake up from sleep mode : wire PIN_POWER to GND to wake up the device.
 * Set PIN_POWER as INPUT_PULLUP to avoid spurious wakeup
 * Wakeup interrupt attached on PIN_POWER, calling power_dummy on wake up.
 */
void power_setup(){
  pinMode(PIN_POWER, INPUT_PULLUP);
  LowPower.attachInterruptWakeup(PIN_POWER, power_dummy, CHANGE);
}

/**
 * @brief Interrupt function, executed when the device wake up from sleep mode.
 * Rename and add some instructions if needed.
 */
void power_dummy(){
};

// Classes Methods
/**
 *  @brief PCA9540B class constructor. Initializes the I2C address.
*/
PCA9540B::PCA9540B(uint8_t address) : _address(address) {}

/** 
 * @brief Initialize the I2C bus 
 */
void PCA9540B::begin() {
  Wire.begin();
}

/**
 *  @brief Select the channel to use on the PCA9540B I²C-bus
 *  @param channel : 0 or 1
*/
void PCA9540B::selectChannel(uint8_t channel) {
  Wire.beginTransmission(_address);
  if (channel == 0) {
    Wire.write(CHANNEL_0);
    #if PLEXER_DEBUG
      Serial.println(F("Channel 0 selected"));
    #endif
  } 
  else if (channel == 1) {
    Wire.write(CHANNEL_1);
    #if PLEXER_DEBUG
      Serial.println(F("Channel 1 selected"));
    #endif
  } 
  else {
    Wire.write(DEFAULT);
    #if PLEXER_DEBUG
      Serial.println(F("default state"));
    #endif
  }
  Wire.endTransmission();
}