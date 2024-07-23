import utime
import struct
import ustruct
import uos  #---- Micro sd
import uasyncio as asyncio
from time import sleep, time
from GPS import GPS
import pyb
from pyb import LED, UART, SDCard
from machine import Pin, SPI, UART
from ulora import TTN, uLoRa
import ujson
from RTC import RTC


async def main():
#---- Enable Main Power Supply
    main_pwr = Pin("MAIN_PWR", Pin.OUT_PP)
    main_pwr.on()
    
#---------------// Peripheral Power //----------------------------------------------------------------------------------------------------
#---- Peripheral Power 
    perif_pwr = Pin("G6", Pin.OUT_PP)
    perif_pwr.on()

#---- Mikrobus Power
    mkbus_pwr = Pin("G4", Pin.OUT_PP)
    mkbus_pwr.on()
#----------------------------------------------------------------------------------------------------------------------------------------------------------

#---- Wait 1s
    utime.sleep(1)    

#---------------// Variables //----------------------------------------------------------------------------------------------------------------------
#---- LED Initialisation
    red = LED(1)
    green = LED(2)
    blue = LED(3)
#---- USER Button pull-up mode
    button = Pin('G0', Pin.IN, Pin.PULL_UP)
#---- Button previous state
    old_button_level = 1
#---- GPS Timeout
    MAX_TIME = 120
#---- GPS Initialisation : UART4
    gps = GPS(4)
#-----------------------------------------------------------------------------------------------------------------------------------------------------------

#---------------// UART //----------------------------------------------------------------------------------------------------------------------------
#--------// Librairy github : https://github.com/fantasticdonkey/uLoRa //--------
#---- UART Communication Boopy/Feather
#---- Selecting UART8 (Uart Debug. Boopy)
    UART_ID = const(8)
#---- Initialisation Port and Speed : 9600bauds
    uart1 = machine.UART(UART_ID, 9600)
#---- Initialisation : 9600bauds, 8 bits, no parity, 1 stop bit
    uart1.init(9600, bits = 8, parity = None, stop = 1)
#-----------------------------------------------------------------------------------------------------------------------------------------------------------

#---- Loop counter
    boucle = 0
    
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

#---- From TTN console for device
#---- LoRa Network configuration
#---------------// Parameters for private gateway (V. Boitier) //----------------------------------------------------
#---- 0xBAC20201
    DEVADDR = bytearray([0xBA, 0xC2, 0x02, 0x01])
    
#---- 0xD57FBCE47FE9BA4B61E7A904C6AE6AA4
    NWKEY = bytearray([0xD5, 0x7F, 0xBC, 0xE4, 0x7F, 0xE9, 0xBA, 0x4B,
                       0x61, 0xE7, 0xA9, 0x04, 0xC6, 0xAE, 0x6A, 0xA4])
                       
#---- 0xA3C07F73E8B374D22EB65429F6F0BC8B
    APP = bytearray([0xA3, 0xC0, 0x7F, 0x73, 0xE8, 0xB3, 0x74, 0xD2,
                     0x2E, 0xB6, 0x54, 0x29, 0xF6, 0xF0, 0xBC, 0x8B])
#----------------------------------------------------------------------------------------------------------------------------------------------------------

#---- Init with hardware address define, server EU
    TTN_CONFIG = TTN(DEVADDR, NWKEY, APP, country="EU")
    FPORT = 1

#---- LoRa initialisation function
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
    '''
#---------------// Debug LoRa //----------------------------------------------------------------------------------------------------------------
#---- Data send on LoRa (test version)
    data = bytearray([0xBA, 0x23, 0x98])
#---- Sending function
    lora.send_data(data, len(data), lora.frame_counter)
    print("data send\n")
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''
    
    
#---------------// Initialisation GPS //------------------------------------------------------------------------------------------------------------------
#---- Set timeout value
    timeout = 0
#---- Create array to store GPS value
    data_list = [0] * 16
#---- nonready GPS
    gps.gps_ok = False
#---- Established conection GPS
#-------- If timeout is reached starting blink red LED
    while gps.gps_ok == False and timeout <= MAX_TIME:
#---- Add one on counter
        timeout += 1
#---- Blinking green LED during searching conection (works normaly)
        green.toggle()
#---- Wait 1s
        await asyncio.sleep(1)
#---- GPS is ok
    if gps.gps_ok == True:
#---- Read date and time and save in data_list
        gps.frame2bin(data_list)
#---- Print GPS time
        print("Heure : " + str(data_list[0]) + ":" + str(data_list[1]) + ":" + str(data_list[2]))
#---- Print GPS date
        print("Date : " + str(data_list[5]) + "/" + str(data_list[4]) + "/" + str(data_list[3]))
        print("------------------------------------------------------")
    else:
        green.low()
        print("GPS fail to connect")
        while True:
            if boucle % 250 == 0:
                red.toggle()
    utime.sleep_ms(1)
    boucle = boucle+1
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    
 #---------------// RTC //-----------------------------------------------------------------------------------------------------------------------------
#---- RTC on SPI 5
    rtc = RTC(5)
#---- Reset all RTC registers
    rtc.reset()
#---- Reset interruption flag and Set external pin to activate STM when alarm
    rtc.clear_alarm()
#---- Set clock value
    rtc.set_clock(data_list[0], data_list[1], data_list[2])
#---- Set calendar value
    rtc.set_calendar(data_list[5], data_list[4], data_list[3])
#---- Set alarm value
    rtc.set_alarm(0, data_list[0], data_list[1] + 1)
    print("Alarme set")
#---- Result variable
    result = [0] * 4
#---- Wait 3s to prevent softlock
    utime.sleep(3)
#----------------------------------------------------------------------------------------------------------------------------------------------------------
 
#---------------// SD //--------------------------------------------------------------------------------------------------------------------
#---- Mount virtual peripheral
#-------- If fail print error message
    try:
        uos.mount(SDCard(), "/sd")
        print("SD Mounted")
    except:
        print("SD Mounting error")
#---- New file variable (unused in debug mode)
    file_name = str(data_list[5]) + "_" + str(data_list[4]) + "_" + str(data_list[3]) + "_" + "data.txt"
    print(file_name)
    
    while True:
        utime.sleep_ms(1)
    machine.deepsleep()
asyncio.run(main())

