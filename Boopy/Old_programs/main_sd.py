'''
BOOPY V2 - version 4.5 du 221009:
Test cycle sommeil/Reveil
'''
import utime
import ustruct
import uasyncio as asyncio
import uos # debug sur micro sd
from pyb import SDCard
from pyb import Pin
from GPS import GPS
from RTC import RTC
from pyb import UART
from pyb import LED
from MCP9808 import MCP
from BME280 import BME
from MPU6050 import MPU
from INA219 import INA

def main_sd():
    main_pwr = Pin("MAIN_PWR", Pin.OUT_PP) 
    main_pwr.on()
    
    utime.sleep(1)
    
    rtc = RTC(5)  # RTC sur le SPI_5
   
    perif_pwr = Pin("PERIF_PWR", Pin.OUT_PP)
    perif_pwr.on() # Alimentation des périphériques dont le GPS
    
    utime.sleep(1)
    
    int_sensor_pwr = Pin("INT_SENSOR_PWR", Pin.OUT_PP)
    int_sensor_pwr.on() # Alimentation des capteurs internes

    ext_sensor_pwr = Pin("EXT_SENSOR_PWR", Pin.OUT_PP)
    ext_sensor_pwr.on() # Alimentation des capteurs externes
    ext_sensor_oe = Pin("EXT_SENSOR_OE", Pin.OUT_PP)
    ext_sensor_oe.on() # Driver I2C On

    click_pwr = Pin("CLICK_PWR", Pin.OUT_PP)
    click_pwr.off()
    
    iridium_pwr = Pin("IRIDIUM_PWR", Pin.OUT_PP)
    iridium_pwr.off()
    
    utime.sleep(1)
    
    buzzer = Pin("BUZZ", Pin.OUT_PP)
    buzzer.off()
    
    chrg_act = Pin("CHRG_ACT", Pin.IN)
    chrg_done = Pin("CHRG_DONE", Pin.IN)
    
    red = LED(1)
    green = LED(2)
    blue = LED(3)
    n = 4
    while n > 0:
        blue.toggle()
        green.toggle()
        red.toggle()
        if buzzer.value() == 0:
            buzzer.on()
        else:
            buzzer.off()
        n -= 1
        utime.sleep(1)  

    utime.sleep(1)
    
    #--- FAIRE LES MESURES --------------------------------
    #--- Init état chargeur
    chrg_act = Pin("CHRG_ACT", Pin.IN)
    chrg_done = Pin("CHRG_DONE", Pin.IN)    
    
    #--- Initialisation des capteurs ------------------------
    error_list = []
    
    #----- GPS
    try:
        gps = GPS(4)  # Initialisation GPS sur l'UART4
    except:
        error_list.append("Error GPS")
        
    #----- Temperature interieure  
    try:
        mcp_int = MCP(4, 0x18)
        mcp_int.set_res(mcp_int.RES_MAX)
        temp_int_ok = True
    except:
        error_list.append("Error Temp Int")
        temp_int_ok = False
        
    #------ Température extérieure
    try:
        mcp_ext = MCP(1, 0x19)
        mcp_ext.set_res(mcp_ext.RES_MAX)
        temp_ext_ok = True
    except:
        error_list.append("Error Temp Ext")
        temp_ext_ok = False
        
    #------ Pression Température Humidité
    try:
        bme = BME(4, 0x76)
        bme.humidity_mode(BME.OVRSAMP_16)
        bme.temperature_mode(BME.OVRSAMP_16)
        bme.pressure_mode(BME.OVRSAMP_16)
        bme.normalmode(BME.HUNDREDTWENTYFIVE_MS)
        bme.filtering(BME.IIR_16)        
        ptu_ok = True
    except:
        error_list.append("Error PTU")
        ptu_ok = False
        
    #------ Centrale inertielle
    try:
        mpu = MPU(4, 0x68)
        
        mpu_ok = True
    except:
        error_list.append("Error MPU")
        mpu_ok = False
        
    #------ Energie panneau solaire
    try:
        solar = INA(4, 0x44)
        solar_ok = True
    except:
        error_list.append("Error Solar")
        solar_ok = False
        
    #------ Energie batterie
    try:
        batt = INA(4, 0x41)
        batt_ok = True
    except:
        error_list.append("Error Battery")
        batt_ok = False
        
    #----- Energie utilisateur
    try:
        user = INA(4, 0x40)
        user_ok = True
    except:
        error_list.append("Error User")
        user_ok = False
        
    print( error_list )    
    
    #=== FAIRE LES MESURES
    data_list = [''] * 25 # Initialisation de la trame de mesures en octets
    print("GPS Localisation")
    #--- Attente de la première trame GPS en moins de 15 minutes
    timeout = 0
    gps.gps_ok = False
    while gps.gps_ok == False and timeout < 900:
        timeout += 1
        green.toggle()
        print(str(timeout), end=" ")
        await asyncio.sleep(1)  

    red.on()
    green.off()
    
    #-- Lecture des données de la trame GPS
    timeout = 0
    gps.gps_ok = False
    while gps.gps_ok == False and timeout < 120:
        timeout += 1
        green.toggle()
        await asyncio.sleep(1)
    gps.frame2ascii(data_list)

    #-- Lecture des données des autres capteurs
    if temp_int_ok == True:
        mcp_int.get_temp_text(data_list, 7)
    red.toggle()    
    if temp_ext_ok == True:
        mcp_ext.get_temp_text(data_list, 8)
    red.toggle()    
    if ptu_ok == True:
        bme.measure_text(data_list, 9, 10)
    red.toggle()
    if mpu_ok == True:
        mpu.get_acc_text(data_list, 11, 12, 13)
        mpu.get_gyro_text(data_list, 14, 15, 16)
    red.toggle()    
    if solar_ok == True:
        solar.energy(data_list, 17, 18)
    red.toggle()    
    if batt_ok == True:
        batt.energy(data_list, 19, 20)
    red.toggle()
    if user_ok == True:
        user.energy(data_list, 21, 22)
    red.toggle()
        
    #--- Status chargeur --------------------------------------------
    iread = 0
    chg_on = 0
    while iread < 10:  #--- Lecture pendant 1 sec
        if chrg_act.value() == 0:
            chg_on += 1
        iread += 1
        utime.sleep_ms(10)                
    #--- Interprétation du résultat
    if chg_on > 0:
        if chg_on == 10:
            #debug.write("Chg\r")
            status = "INCH"
        else:
            #debug.write("Low\r")
            status = "LOCH"
    else:
        #debug.write("NoCh\r")
        status = "NOCH"

    if chrg_done.value() == 0:
        #debug.write("Done\r")
        status = "CHDO"                
    data_list[23] = status

    frame = "$BOOPY"
    for item in data_list:
        frame += item + ';'
    frame += '%'
    
    red.off()
    
    # ============ F I L E S ================
    blue.on()
    try:
        uos.mount(SDCard(), "/sd")
    except:
        print("Error SD mount")
        
    #--- Lire le numéro du fichier
    try:
        file = open("/flash/sd_file_cnt.txt", "r")
        ifile = int(file.readline())
        file.close()
    except:
        ifile = 0
    print("num fichier : " + str(ifile) + '\r')
    filenamed = "/sd/data_" + str(ifile) + ".csv"
    
    #--- Lire le numéro de la trame. 
    try:
        file = open("/flash/sd_frame_cnt.txt", "r")
        iframe = int(file.readline())
        file.close()
    except:
        iframe = 0    
    iframe +=1
    print("num frame : " + str(iframe) + '\r')
    
    #--- Ecrire une trame dans le fichier de la SD
    try:
        file = open(filenamed, "a")
        if iframe == 1:
            file.write("Synchro;Heure;Date;Latitude;NS;Longitude;EW;Temp_Int;Temp_Ext;Pression;Humid;AccX;AccY;AccZ;GyrX;GyrY;GyrZ;USol;ISol;UBat;IBat;UUsr;IUsr;Chg\n")    
        file.write(frame + '\r')
        file.close()
    except:
        print("erreur ecriture fichier data")         
    
    if iframe == 24:
        iframe = 0
        ifile += 1
        try:
            file = open("/flash/sd_file_cnt.txt", 'w')
            file.write(str(ifile) + '\n')
            file.close()
        except:
            print("erreur ecriture fichier file_cnt")
            
    #--- Ecrire le numero de la trame
    try:
        file = open("/flash/sd_frame_cnt.txt", "w")
        file.write(str(iframe) + '\n')
        file.close()
    except:
        print("Erreur ecriture iframe")
        
    blue.off()    
    
    #--- Programmation de l'alarme de reveil de la RTC
    #--- Read minutes on GPS
    print("GPS Minutes")
    timeout = 0
    gps.gps_ok = False
    while gps.gps_ok == False and timeout < 120:
        timeout += 1
        green.toggle()
        print(str(timeout), end=" ")
        await asyncio.sleep(1)
    green.off()
    minutes = gps.read_minutes()  # gps.read_minutes()
    
    print("Minutes = " + str(minutes))
    #--- Calcul du temps de sommeil en minutes
   
    if minutes > 0:
        wakeup = 60 - minutes
    else:
        wakeup = 59        
    
    rtc.clear_alarm()
    rtc.set_RTC()
    rtc.set_alarm(1, 0, wakeup) # jour 1, 0h, wakeup_minutes

    try:
        file = open("/sd/debug.txt", "a")
        file.write(str(wakeup) + '\n')
        file.close()
    except:
        print("Erreur ecriture debug")
        
    utime.sleep(2)  # Attente de 2 secondes avant de...couper'''
BOOPY V2 - version 4.5 du 221009:
Test cycle sommeil/Reveil
'''
import utime
import ustruct
import uasyncio as asyncio
import uos # debug sur micro sd
from pyb impor