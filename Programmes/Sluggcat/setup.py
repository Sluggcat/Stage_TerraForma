from machine import SPI, Pin
from micropython_rfm9x import *

RADIO_FREQ_MHZ = 868.0

# config. pins du module LoRA
CS  = Pin(33,Pin.OUT)
RST = Pin(32,Pin.OUT)

spi = SPI(
    2, baudrate=1000000, polarity=0, phase=0, bits=8, firstbit=0,
    sck=Pin(5),
    mosi=Pin(18),
    miso=Pin(19)
)

# init_radio LoRa
rfm9x = RFM9x(spi, CS, RST, RADIO_FREQ_MHZ)

# radio power : 13 typique, RFM95 23 max
rfm9x.ty = 23

