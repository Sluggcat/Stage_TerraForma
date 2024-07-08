# main.py

import utime
import config
from machine import Pin
import ubinascii

def main():
    start_time = utime.ticks_ms()
    print("===\nProgram start\n")
    
    # Turn LED on for the duration of the program
    led = Pin(config.LED_PIN, Pin.OUT, value=1)
    print("---\nSx1276 config")
    
    # LoRaWAN / TTN send
    lora = config.initialize_lora()
    
    # Test sending data
    data = bytearray([0xA5, 0xB3, 0x80])
    
    print("Sending packet...", lora.frame_counter, ubinascii.hexlify(data), "\n")
    lora.send_data(data, len(data), lora.frame_counter)
    
    lora.frame_counter += 1
    print(len(data), "bytes sent!\nframe_counter: ", lora.frame_counter)
    
    utime.sleep_ms(config.PROGRAM_WAIT_MS)
    led.off()
    print("\nEND\n===")

if __name__ == "__main__":
    main()
