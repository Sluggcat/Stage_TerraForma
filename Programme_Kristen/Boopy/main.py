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
import data_transmitter


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
        blue.toggle()
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
    
    #while True:
    utime.sleep_ms(1)
    machine.deepsleep()
asyncio.run(main())

