# boot.py -- run on boot-up

import machine
import pyb
main_pwr = pyb.Pin("MAIN_PWR", pyb.Pin.OUT_PP) 
main_pwr.on()
pyb.country('US') # ISO 3166-1 Alpha-2 code, eg US, GB, DE, AU
pyb.main('main.py') # main script to run after this one
#pyb.usb_mode('VCP+MSC') # act as a serial and a storage device
#pyb.usb_mode('VCP+HID') # act as a serial device and a mouse