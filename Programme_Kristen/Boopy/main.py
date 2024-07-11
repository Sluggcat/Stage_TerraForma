import utime
import struct
import ustruct
import uos  # Micro sd
import uasyncio as asyncio
from time import sleep, time
from GPS import GPS
import pyb
from pyb import LED, UART, SDCard
from machine import Pin, SPI, UART
from ulora import TTN, uLoRa
import ujson
from RTC import RTC

#---------------// Variables //----------------------------------------------------------------------------------------------------------------------
red = LED(1)
green = LED(2)
bouton = Pin('G0', Pin.IN, Pin.PULL_UP)
old_button_level = 1
boucle = 0
MAX_TIME = 120
gps = GPS(4)
data_list = [0] * 16
#-----------------------------------------------------------------------------------------------------------------------------------------------------------



#---------------// UART //----------------------------------------------------------------------------------------------------------------------------
#--------// Github de la librairie : https://github.com/fantasticdonkey/uLoRa //--------
#Parametrage liaison UART
UART_ID = const(8) #Selection UART8 (Uart Debug. Boopy)
uart1 = machine.UART(UART_ID, 9600)  
uart1.init(9600, bits = 8, parity = None, stop = 1)  #Initialisation : 9600bauds, mot de 8 bits, pas de bit de parite, 1 bit de stop
#-----------------------------------------------------------------------------------------------------------------------------------------------------------



#---------------// Power Peripherique //----------------------------------------------------------------------------------------------------
# alim. périphériques 
perif_pwr = Pin("G6", Pin.OUT_PP)
perif_pwr.on()

# alim. mkbus
mkbus_pwr = Pin("G4", Pin.OUT_PP)
mkbus_pwr.on()
#----------------------------------------------------------------------------------------------------------------------------------------------------------


async def main():
    boucle = 0
    
    '''
#---------------// RTC //------------------------------------------------------------------------------------------------------------------------------
    rtc_pin = Pin("F6", Pin.OUT_PP)
    rtc_pin.high()
    rtc = pyb.SPI(5)
    mode = SPI.CONTROLLER()
    rtc.init(mode, bauderate = 300000, polarity = 1, phase = 0, bits = 8)
    date = bytearray(4)
    cmd = bytearray(2)
    cmd[0] = 0x11  # bit7=0 : écriture - bits0-3=1 : à partir l'@ 0x01 (Control_2)
    cmd[1] = 0x02  # set AIE high clear AF
    print(date)
    rtc_pin.low()
    rtc.send(cmd, timeout = 3000)
    cmd = bytearray(9)
    cmd[0] = 0x12  # bit7=0 : écriture - bits0-3=2 : à partir de l'@ 0x02 (secondes)
    cmd[1] = self.int_to_bcd(0)  # seconds reg:0x02
    cmd[2] = self.int_to_bcd(0)  # minutes reg:0x03
    cmd[3] = self.int_to_bcd(0)  # heures reg:0x04
    cmd[4] = self.int_to_bcd(1)  # day
    cmd[5] = self.int_to_bcd(0)  # ---- day of week
    cmd[6] = self.int_to_bcd(1)  # Month
    cmd[7] = self.int_to_bcd(20) # year
    cmd[8] = self.int_to_bcd(0)  # ----------------
    rtc.send(cmd, timeout = 3000)
    rtc.recv(date, timeout = 2000)
    print(date)
    print("--------------------------------------")
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''

    '''
#---------------// RTC //-----------------------------------------------------------------------------------------------------------------------------
    rtc = RTC(5)
    #rtc.clear_alarm()
    #rtc.set_RTC()
    #rtc.set_clock(10,00,30)
    rtc.set_calendar(07,08,2024)
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''

    '''
#---------------// LoRa //----------------------------------------------------------------------------------------------------------------------------
    #Paramétrage LoRa pour Boopy
    # Refer to device pinout / schematics diagrams for pin details
    LORA_CS = 'E4'           #Pin CS sur E4
    LORA_SCK = 'E12'      #Pin SCK sur E12
    LORA_MOSI = 'E6'      #Pin MOSI sur E6
    LORA_MISO = 'E5'      #Pin MISO sur E5
    LORA_IRQ = 'F3'         #Pin IRQ sur F3 - Interruption
    LORA_RST = 'F11'       #Pin RST sur F11
    LORA_DATARATE = "SF9BW125"  # Choose from several available

    # From TTN console for device
    #Adresse de configuration réseau LoRa
    DEVADDR = bytearray([0xBA, 0xC2, 0x02, 0x01])                           #BAC20201
    NWKEY = bytearray([0xD5, 0x7F, 0xBC, 0xE4, 0x7F, 0xE9, 0xBA, 0x4B,
                       0x61, 0xE7, 0xA9, 0x04, 0xC6, 0xAE, 0x6A, 0xA4])       #D57FBCE47FE9BA4B61E7A904C6AE6AA4
    APP = bytearray([0xA3, 0xC0, 0x7F, 0x73, 0xE8, 0xB3, 0x74, 0xD2,
                     0x2E, 0xB6, 0x54, 0x29, 0xF6, 0xF0, 0xBC, 0x8B])           #A3C07F73E8B374D22EB65429F6F0BC8B
                     
    #Initialisation de la connexion avec les adresses choisies, reseau Europe
    TTN_CONFIG = TTN(DEVADDR, NWKEY, APP, country="EU")
    FPORT = 1

    #Initialisation de l'objet lora pour la communication
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
    #Fin parametrage

#---------------// Debug LoRa //----------------------------------------------------------------------------------------------------------------

    #Data a envoyer sur LoRa
    data = bytearray([0xBA, 0x23, 0x98])
    # ...Then send data as bytearray
    lora.send_data(data, len(data), lora.frame_counter)
    print("data send\n")
    #print(buf)
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''

    '''
#---------------// Debug JSON //---------------------------------------------------------------------------------------------------------------
    aDict = {
      "squadName": "Super hero squad",
      "homeTown": "Metro City",
      "formed": 2016,
      "secretBase": "Super tower",
      "active": True,
      "members": [
        {
          "name": "Molecule Man",
          "age": 29,
          "secretIdentity": "Dan Jukes",
          "powers": ["Radiation resistance", "Turning tiny", "Radiation blast"]
        },
        {
          "name": "Madame Uppercut",
          "age": 39,
          "secretIdentity": "Jane Wilson",
          "powers": [
            "Million tonne punch",
            "Damage resistance",
            "Superhuman reflexes"
          ]
        },
        {
          "name": "Eternal Flame",
          "age": 1000000,
          "secretIdentity": "Unknown",
          "powers": [
            "Immortality",
            "Heat Immunity",
            "Inferno",
            "Teleportation",
            "Interdimensional travel"
          ]
        }
      ]
    }

    jsonString = ujson.dumps(aDict)
    jsonFile = open("data.json", "w")
    jsonFile.write(jsonString)
    jsonFile.close()

    fileObject = open("data.json", "r")
    jsonContent = fileObject.read()
    obj_python = ujson.loads(jsonContent)
    print(obj_python['members'][2]['name'])
    print(" : ")
    print(obj_python['members'][2]['age'])
    fileObject.close()

    print("Done !")
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''
    

#---------------// Debug SD //--------------------------------------------------------------------------------------------------------------------
    try:
        uos.mount(SDCard(), "/sd")
    except:
        print("Erreur lecture SD")
    
    ifile = 1
    
    try:
        file = open("/sd/data1.txt", "w")
        file.write("Daaaaaaaamme les gens")
        file.close()
    except:
        print("Erreur ecriture fichier")
        
    try:
        file = open("/sd/data1.txt", "r")
        context = file.read()
        file.close()
        print(str(context))
    except:
        print("Erreur lecture")
#----------------------------------------------------------------------------------------------------------------------------------------------------------
        
    while True:
        '''
#---------------// Debug Bouton //-------------------------------------------------------------------------------------------------------------
        #Debug avec bouton poussoir
        button_level = bouton.value()
        if button_level != old_button_level:
            print("Niveau logique du bouton : ", button_level)
            old_button_level = button_level
            # Change l'état de la LED
            green.toggle()
#----------------------------------------------------------------------------------------------------------------------------------------------------------
        '''
        
        '''
#---------------// Debug UART //---------------------------------------------------------------------------------------------------------------
        if boucle % 1000 == 0:        
            #print("running...." + str(UART_ID) + "\n")
            #print(str(uart1.write('44\n')) + " Bits ecrit")
            #uart1.write('Saluut\n')
            green.toggle()
        message_received = uart1.read()
        if not (message_received is None) :
            #message_decoded = struct.unpack('f',message_received)
            print(str(time()) + " : Message reçu : " + message_received.decode("utf-8"))
        utime.sleep_ms(1)
        boucle = boucle+1
#----------------------------------------------------------------------------------------------------------------------------------------------------------
        '''
        
        '''
#---------------// Debug RTC //------------------------------------------------------------------------------------------------------------------
        if boucle % 1000 == 0:
            #heure = rtc.read_clock()
            #print(date)
            #print(str(heure))
            #print("Date : " + str(date[3]) + "/" + str(date[2]) + "/" + str(date[0]) + str(date[1]))
            #print("Heure : " + str(heure[2]) + ":" + str(heure[1]) + ":" + str(heure[0]))
            #print("--------------------------------------")
            rat = 1
        utime.sleep_ms(1)
        boucle = boucle+1
#----------------------------------------------------------------------------------------------------------------------------------------------------------
        '''
        
        '''
#---------------// Debug GPS //------------------------------------------------------------------------------------------------------------------
        timeout = 0
        gps.gps_ok = False
        while gps.gps_ok == False and timeout < MAX_TIME:
            timeout += 1
            green.tog

