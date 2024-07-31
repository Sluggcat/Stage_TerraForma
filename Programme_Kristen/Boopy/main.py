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
from data_transmitter import DataTransmitter, UARTReceiver
import config


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
    MAX_TIME = 30
#---- GPS Initialisation : UART4
    gps = GPS(4)
    boucle = 0
#-----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''
#---------------// Initialisation GPS //------------------------------------------------------------------------------------------------------------------
    timeout = 0
    data_list = [0] * 16
    gps.gps_ok = False
    while gps.gps_ok == False and timeout <= MAX_TIME:
        timeout += 1
        blue.toggle()
        await asyncio.sleep(1)
    if gps.gps_ok == True:
        gps.frame2bin(data_list)
        print("Heure : " + str(data_list[0]) + ":" + str(data_list[1]) + ":" + str(data_list[2]))
        print("Date : " + str(data_list[5]) + "/" + str(data_list[4]) + "/" + str(data_list[3]))
        print("------------------------------------------------------")
    else:
        blue.off()
        print("GPS fail to connect")
        while True:
            if boucle % 250 == 0:
                red.toggle()
            utime.sleep_ms(1)
            boucle = boucle+1
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''
#---------------// RTC //-----------------------------------------------------------------------------------------------------------------------------
    rtc = RTC(5)
    rtc.reset()
    rtc.clear_alarm()
    '''
    rtc.set_clock(data_list[0], data_list[1], data_list[2])
    rtc.set_calendar(data_list[5], data_list[4], data_list[3])
    rtc.set_alarm(0, data_list[0], data_list[1] + 2)
    '''
    rtc.set_clock(00, 00, 00)
    rtc.set_alarm(00, 00, 02)
    print("Alarme set")
    result = [0] * 4
    utime.sleep(3)
#----------------------------------------------------------------------------------------------------------------------------------------------------------
     
#---------------// SD //--------------------------------------------------------------------------------------------------------------------
    try:
        uos.mount(SDCard(), "/sd")
        print("SD Mounted")
    except:
        print("SD Mounting error")
        boucle = 0
        while True:
            if boucle % 500 == 0:
                red.toggle()
                blue.toggle()
            utime.sleep_ms(1)
            boucle = boucle+1
            
    file_name = "test_data.txt"#--str(data_list[5]) + "_" + str(data_list[4]) + "_" + str(data_list[3]) + "_" + "data.txt"
    file = open("/sd/" + file_name, "w")
    file.close()
    print(file_name)
            
    
    while True:
        #---- Data transmitter
        data = DataTransmitter()
        data.run()
        #floats = [25.64, 1024.0, 12.35, 7.65, 3.14, 250.95, 123.0, 258.0, 365.0, 489.0, 698.0, 720.0, 810.0, 963.0, 1056.0, 1204.0, 1566.0, 1900.0]

        try:
            file = open("/sd/" + file_name, "r")
            cursor = len(file.read())
            file.close()
            file = open("/sd/" + file_name, "w")
            file.seek(cursor)
            file.write(data.getValues())
            file.write("\n")
            file.close()
            file = open("/sd/" + file_name, "r")
            print(file.read())
            file.close()
            utime.sleep_ms(100)
        except:
            print("Erreur fichier")
            boucle = 0
            while True:
                if boucle % 1000 == 0:
                    blue.toggle()
                utime.sleep_ms(1)
                boucle = boucle+1
        utime.sleep_ms(1)
    #machine.deepsleep()
asyncio.run(main())