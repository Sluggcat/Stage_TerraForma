from machine import UART
from ulora import TTN, uLoRa

#---------------// LoRa //----------------------------------------------------------------------------------------------------------------------------
#---- Setting for Boopy's LoRa device
#---- Refer to device pinout / schematics diagrams for pin details
#---- Pin CS sur E4
LORA_CS = 'E4'
#---- Pin SCK sur E12
LORA_SCK = 'E12'
#---- Pin MOSI sur E6
LORA_MOSI = 'E6'
#---- Pin MISO sur E5
LORA_MISO = 'E5'
#---- Pin IRQ sur F3 - Interruption
LORA_IRQ = 'F3'
#---- Pin RST sur F11
LORA_RST = 'F11'
#---- Communication settings
LORA_DATARATE = "SF9BW125"

#---- LoRa Network configuration
#---- 0xBAC20201
DEVADDR = bytearray([0xBA, 0xC2, 0x02, 0x01])

#---- 0xD57FBCE47FE9BA4B61E7A904C6AE6AA4
NWKEY = bytearray([
    0xD5, 0x7F, 0xBC, 0xE4, 0x7F, 0xE9, 0xBA, 0x4B,
    0x61, 0xE7, 0xA9, 0x04, 0xC6, 0xAE, 0x6A, 0xA4
    ])
                   
#---- 0xA3C07F73E8B374D22EB65429F6F0BC8B
APP = bytearray([
    0xA3, 0xC0, 0x7F, 0x73, 0xE8, 0xB3, 0x74, 0xD2,
    0x2E, 0xB6, 0x54, 0x29, 0xF6, 0xF0, 0xBC, 0x8B
    ])
#----------------------------------------------------------------------------------------------------------------------------------------------------------
#---- Init with hardware address define, server EU
TTN_CONFIG = TTN(DEVADDR, NWKEY, APP, country="EU")
FPORT = const(1)

#---- LoRa initialisation function
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
        fport=FPORT
    )
    return lora

#---------------// UART //----------------------------------------------------------------------------------------------------------------------------
#---- UART Communication Boopy/Feather
def initialize_UART():
#---- Selecting UART8 (Uart Debug. Boopy)
    UART_ID = const(8)
#---- Initialisation Port and Speed : 9600bauds
    uart1 = machine.UART(UART_ID, 9600)
#---- Initialisation : 9600bauds, 8 bits, no parity, 1 stop bit
    uart1.init(9600, bits = 8, parity = None, stop = 1)
    return uart1
#-----------------------------------------------------------------------------------------------------------------------------------------------------------


