# config.py
from machine import I2C, Pin
from ulora import TTN, uLoRa

# Time constants
PROGRAM_LOOP_MS = const(600000)
PROGRAM_WAIT_MS = const(3000)

# ESP32 Dev.board
# SPI pins
LORA_CS = const(33)
LORA_SCK = const(5)
LORA_MOSI = const(18)
LORA_MISO = const(19)
LORA_IRQ = const(4)
LORA_RST = const(32)
LORA_FPORT = const(1)
LORA_DATARATE = "SF8BW125"

# Uart pins
RX_PIN = const(16)
TX_PIN = const(17)

# Onboard LED
LED_PIN = const(2)

# The Things Network (TTN) device details
# Device address : 4 bytes MSB 
TTN_DEVADDR = bytearray([0xBA, 0xC2, 0x02, 0x00])

# Network session key, 16 Bytes, MSB
TTN_NWKEY = bytearray([
    0x9F, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x70, 0x81,
    0x92, 0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07
])

# Application session key, 16 Bytes, MSB
TTN_APP = bytearray([
    0xF2, 0xD4, 0xE7, 0xC8, 0xA1, 0xB2, 0xC3, 0xD4,
    0xE5, 0xF6, 0x07, 0x8A, 0x9B, 0x0C, 0x1D, 0x2E
])

TTN_CONFIG = TTN(TTN_DEVADDR, TTN_NWKEY, TTN_APP, country="EU")

def initialize_lora():
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
    return lora
