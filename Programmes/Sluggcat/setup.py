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

# Modifier ce dictionnaire pour config. ABP de LoRaWAN
ttn_config ={
    # Adresse du dispositif
    'devadd': bytearray([0x00, 0x00, 0x00, 0x00]),
    
    # Clé de session du réseau
    'nwkey': bytearray([
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    ]),
    
    # Clé de session de l'application
    'app': bytearray([
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    ]),
    
    # Région
    'country': 'EU',
}


