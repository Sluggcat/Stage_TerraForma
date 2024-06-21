import utime
from time import sleep
import pyb
from pyb import LED
from machine import Pin, SPI
from ulora import TTN, uLoRa

# alim. périphériques (mkbus)
mkbus_pwr = Pin("G4", Pin.OUT_PP)
mkbus_pwr.on()

# alim. périphériques (mkbus)
mkbus_pwr = Pin("G6", Pin.OUT_PP)
mkbus_pwr.on()

# Refer to device pinout / schematics diagrams for pin details
LORA_CS = 'E4'
LORA_SCK = 'E12'
LORA_MOSI = 'E6'
LORA_MISO = 'E5'
LORA_IRQ = 'F3'
LORA_RST = 'F11'
LORA_DATARATE = "SF9BW125"  # Choose from several available
# From TTN console for device
DEVADDR = bytearray([0xBA, 0xC2, 0x02, 0x01])               #BAC20201
NWKEY = bytearray([0xD5, 0x7F, 0xBC, 0xE4, 0x7F, 0xE9, 0xBA, 0x4B,
                   0x61, 0xE7, 0xA9, 0x04, 0xC6, 0xAE, 0x6A, 0xA4]) #D57FBCE47FE9BA4B61E7A904C6AE6AA4
APP = bytearray([0xA3, 0xC0, 0x7F, 0x73, 0xE8, 0xB3, 0x74, 0xD2,
                 0x2E, 0xB6, 0x54, 0x29, 0xF6, 0xF0, 0xBC, 0x8B])   #A3C07F73E8B374D22EB65429F6F0BC8B
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

time_to_start = utime.ticks_us()
data = bytearray([0xBA, 0x50, 0xEF, 0x10, 0x22, 0xFF, 0x87, 0x85, 0xAA, 0x98])
# ...Then send data as bytearray
lora.send_data(data, len(data), lora.frame_counter)
end_time = utime.ticks_us()
print("data send\n")
total_time = (end_time - time_to_start) / 1000
print('Temps passé : ' + str(total_time) + 'ms')
#print(buf)

green = LED(2)

bouton = Pin('G0', Pin.IN, Pin.PULL_UP)

old_button_level = 1

'''while True:
    button_level = bouton.value()
    if button_level != old_button_level:
        print("Niveau logique du bouton : ", button_level)
        old_button_level = button_level
        # Change l'état de la LED
        green.toggle()
            
    print("running....")
    utime.sleep_ms(1000)'''
    



