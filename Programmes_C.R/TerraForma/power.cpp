#include "power.h"

/**
 * @brief Setup function to wake up from sleep mode : wire PIN_POWER to GND to wake up the device.
 * Set PIN_POWER as INPUT_PULLUP to avoid spurious wakeup
 * Attach a wakeup interrupt on PIN_POWER, calling power_dummy on wake up.
 */
void power_setup(){
  pinMode(PIN_POWER, INPUT_PULLUP);
  LowPower.attachInterruptWakeup(PIN_POWER, power_dummy, CHANGE);
}

/**
 * @brief Interrupt function, executed when the device wake up from sleep mode.
 * add some instructions if needed.
 */
void power_dummy(){
};