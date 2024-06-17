import utime
from machine import I2C, Pin, deepsleep
import ubinascii

from ulora import TTN, uLoRa

# ESP32 development board SPI pins
LORA_CS = const(33)
LORA_SCK = const(5)
LORA_MOSI = const(18)
LORA_MISO = const(19)
LORA_IRQ = const(4)
LORA_RST = const(32)
LORA_DATARATE = "SF8BW125"
LORA_FPORT = const(1)

# Onboard LED
LED_PIN = const(2)

# The Things Network (TTN) device details (available in TTN console)
# TTN device address, 4 Bytes, MSB (REPLACE WITH YOUR OWN!!!)
TTN_DEVADDR = bytearray([0xBA, 0xC2, 0x02, 0x00])

# TTN network session key, 16 Bytes, MSB (REPLACE WITH YOUR OWN!!!)
TTN_NWKEY = bytearray([
        0x9F, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x70, 0x81,
        0x92, 0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07])

# TTN application session key, 16 Bytes, MSB (REPLACE WITH YOUR OWN!!!)
TTN_APP = bytearray([
        0xF2, 0xD4, 0xE7, 0xC8, 0xA1, 0xB2, 0xC3, 0xD4,
        0xE5, 0xF6, 0x07, 0x8A, 0x9B, 0x0C, 0x1D, 0x2E])

TTN_CONFIG = TTN(TTN_DEVADDR, TTN_NWKEY, TTN_APP, country="EU")

# Additional configurations
PROGRAM_LOOP_MS = const(600000)
PROGRAM_WAIT_MS = const(3000)

def main():
    start_time = utime.ticks_ms()
    print("===\nProgramm start\n")
    # Turn LED for the duration of the program
    led = Pin(LED_PIN, Pin.OUT, value=1)
    print("---\nSx1276 config")
    
    # LoRaWAN / TTN send
    lora = uLoRa(
            cs=LORA_CS,
            sck=LORA_SCK,
            mosi=LORA_MOSI,
            miso=LORA_MISO,
            irq=LORA_IRQ,
            rst=LORA_RST,
            ttn_config=TTN_CONFIG,
            datarate=LORA_DATARATE,
            fport=LORA_FPORT
    )
    # test température du module
    temp = lora.get_temp()
    print( "\ntemp = ", temp )
    
    # test lire version reg.
    reg_vers = lora._read_u8(0x42)
    print("version : ", reg_vers )
    print("---")
    data = bytearray ([0xA5, 0xB3, 0x80])
    
    #test émission
    print("Sending packet...", lora.frame_counter, ubinascii.hexlify(data), "\n")
    lora.send_data(data, len(data), lora.frame_counter)
    
    lora.frame_counter += 1
    print(len(data), "bytes sent!`\nframe_counter: ",lora.frame_counter)
    


    utime.sleep_ms(PROGRAM_WAIT_MS)
    led.off()
    print("\nEND\n===")


if __name__ == "__main__":
    main()