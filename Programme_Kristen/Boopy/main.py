import utime
from time import sleep
import pyb
from machine import Pin, SPI
from ulora import TTN, uLoRa

# alim. périphériques (mkbus)
mkbus_pwr = Pin("G4", Pin.OUT_PP)
mkbus_pwr.on()

CHIP_SELECT = 'E4'
chip_select = Pin(CHIP_SELECT, Pin.OUT)
chip_select.on()

spi = SPI(4)
spi.init()
sleep(1)

BUFFER = bytearray(2)
buf = bytearray(2)
address = const(0x42)
BUFFER[0] = address & 0x7F
chip_select.off()
spi.write(BUFFER[0:1])
spi.readinto(buf)
chip_select.on()

'''# Refer to device pinout / schematics diagrams for pin details
LORA_CS = 'E4'
LORA_SCK = 'E12'
LORA_MOSI = 'E6'
LORA_MISO = 'E5'
LORA_IRQ = 'F3'
LORA_RST = 'F11'
LORA_DATARATE = "SF9BW125"  # Choose from several available
# From TTN console for device
DEVADDR = bytearray([0x00, 0x00, 0x00, 0x00])
NWKEY = bytearray([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
APP = bytearray([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
TTN_CONFIG = TTN(DEVADDR, NWKEY, APP, country="EU")
FPORT = 1
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
# ...Then send data as bytearray
lora.send_data(data, len(data), lora.frame_counter)'''
print("data send")
print(buf)

