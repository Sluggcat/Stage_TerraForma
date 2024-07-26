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
#---- Alimentation generale
    main_pwr = Pin("MAIN_PWR", Pin.OUT_PP)
    main_pwr.on()

#---- Attente 1s
    utime.sleep(1)    

#---------------// Variables //----------------------------------------------------------------------------------------------------------------------
#---- Led rouge et verte
    red = LED(1)
    green = LED(2)
#---- Bouton USER en mode pull-up
    bouton = Pin('G0', Pin.IN, Pin.PULL_UP)
#---- Etat precedent du bouton
    old_button_level = 1
#---- Temps max pour le timeout des commandes GPS
    MAX_TIME = 120
#---- Initialisation du GPS : UART 4 de Boopy
    gps = GPS(4)
#-----------------------------------------------------------------------------------------------------------------------------------------------------------



#---------------// UART //----------------------------------------------------------------------------------------------------------------------------
#--------// Github de la librairie : https://github.com/fantasticdonkey/uLoRa //--------
#---- Parametrage liaison UART Boopy/Feather
#---- Selection UART8 (Uart Debug. Boopy)
    UART_ID = const(8)
#---- Selection de la vitesse de communication : 9600bauds
    uart1 = machine.UART(UART_ID, 9600)
#---- Initialisation : 9600bauds, mot de 8 bits, pas de bit de parite, 1 bit de stop
    uart1.init(9600, bits = 8, parity = None, stop = 1)
#-----------------------------------------------------------------------------------------------------------------------------------------------------------



#---------------// Power Peripherique //----------------------------------------------------------------------------------------------------
#---- Alim. périphériques 
    perif_pwr = Pin("G6", Pin.OUT_PP)
    perif_pwr.on()

#---- Alim. mikrobus
    mkbus_pwr = Pin("G4", Pin.OUT_PP)
    mkbus_pwr.on()
#----------------------------------------------------------------------------------------------------------------------------------------------------------


#---- Compteur pour la boucle
    boucle = 0

#---------------// RTC //-----------------------------------------------------------------------------------------------------------------------------
#---- Bibliotheque RTC fournie semble ne pas fonctionner
#---- Initialisation du module RTC sur le SPI 5
    rtc = RTC(5)
#---- Remise au parametre de base du module
    rtc.reset()
#---- Reinitialisation du drapeau d'alarme et activation de la sortie interruption
    rtc.clear_alarm()
#---- Initialisation de l'horloge
    rtc.set_clock(9, 40, 00)
#---- Initialisation du calendrier
    rtc.set_calendar(16, 07, 24)
#---- Creation d'une alarme a l'heure souhaitee
    rtc.set_alarm(0, 9, 41)
    print("Alarme set")
#----// Debug de l'initialisation //-------------------------------------------------
    rtcspi = SPI(5)
    cs = pyb.Pin("F6", pyb.Pin.OUT_PP)
    registre = bytearray([0x99])
    cs.high()
    rtcspi.write(registre)
    rtcspi.readinto(registre)
    cs.low()
    print("Valeur registre alarme : " + str(registre))
    print("------------------------------------------------------------------")

#---- Variable pour enregistrer les valeurs RTC
    result = [0] * 4
#---- Tempo pour eviter le soft lock boopy
    utime.sleep(3)
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    
    '''
#---------------// LoRa //----------------------------------------------------------------------------------------------------------------------------
#---- Paramétrage LoRa pour Boopy
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

#---- Initialisation du format de la communication LoRa
    LORA_DATARATE = "SF9BW125"

#---- From TTN console for device
#---- Adresse de configuration réseau LoRa
#---------------// Parametres pour la gateway prive V. Boitier //----------------------------------------------------
#---- 0xBAC20201
    DEVADDR = bytearray([0xBA, 0xC2, 0x02, 0x01])
    
#---- 0xD57FBCE47FE9BA4B61E7A904C6AE6AA4
    NWKEY = bytearray([0xD5, 0x7F, 0xBC, 0xE4, 0x7F, 0xE9, 0xBA, 0x4B,
                       0x61, 0xE7, 0xA9, 0x04, 0xC6, 0xAE, 0x6A, 0xA4])
                       
#---- 0xA3C07F73E8B374D22EB65429F6F0BC8B
    APP = bytearray([0xA3, 0xC0, 0x7F, 0x73, 0xE8, 0xB3, 0x74, 0xD2,
                     0x2E, 0xB6, 0x54, 0x29, 0xF6, 0xF0, 0xBC, 0x8B])
#----------------------------------------------------------------------------------------------------------------------------------------------------------

#---- Initialisation de la connexion avec les adresses choisies, reseau Europe
    TTN_CONFIG = TTN(DEVADDR, NWKEY, APP, country="EU")
    FPORT = 1

#---- Initialisation de l'objet lora pour la communication
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

#---- Fin parametrage

#---------------// Debug LoRa //----------------------------------------------------------------------------------------------------------------
#---- Data a envoyer sur LoRa (a modifier pour faire les tests)
    data = bytearray([0xBA, 0x23, 0x98])
#---- Envoie des donnees
    lora.send_data(data, len(data), lora.frame_counter)
    print("data send\n")
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''

    '''
#---------------// Debug JSON //---------------------------------------------------------------------------------------------------------------
#---- Création d'un dictionnaire JSon en dur dans le programme
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
#---- Fin de la définition du dictionnaire

#---- Ecriture du dictionnaire dans un fichier JSon
#-------- Mise sur un tampon du dictionnaire
    jsonString = ujson.dumps(aDict)
#-------- Ouverture du fichier, mode ecriture
    jsonFile = open("data.json", "w")
#-------- Ecriture dans le fichier .json
    jsonFile.write(jsonString)
#-------- Fermeture fichier
    jsonFile.close()

#---- Lecture et affichage des donnees recuperees depuis un fichier JSon
#-------- Overture du fichier .json, lecture seulement
    fileObject = open("data.json", "r")
#-------- Lecture du document
    jsonContent = fileObject.read()
#-------- Chargement du contenu dans un objet
    obj_python = ujson.loads(jsonContent)
#-------- Recherche avec les paramètre souhaites et affichage
    print(obj_python['members'][2]['name'])
    print(" : ")
    print(obj_python['members'][2]['age'])
#-------- Fermeture du fichier
    fileObject.close()

    print("Done !")
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''
    
    '''
#---------------// Debug SD //--------------------------------------------------------------------------------------------------------------------
#---- Montage logiciel du peripherique SD
#-------- Si reussi pas de message sinon affichage message d'erreur
    try:
        uos.mount(SDCard(), "/sd")
    except:
        print("Erreur lecture SD")

#---- Création d'une variable pour le nom du fichier (non utilise dans le debug)
    ifile = 1
    
#---- Ecriture dans le fichier data1.txt
#-------- Renvoie un message d'erreur si une action ne reussie pas
    try:
#-------- Overture du fichier en ecriture
        file = open("/sd/data1.txt", "w")
#-------- Ecriture du fichier avec le texte souhaite
        file.write("Yo tout le monde c'est david lafarge Pokemon\n")
#-------- Fermeture du fichier
        file.close()
        utime.sleep(1)
#-------- Ecriture dans le fichier mais en ecrasant des donnees
        file = open("/sd/data1.txt", "w")
#-------- Bouge le cursuer avant l'ecriture
        file.seek(17)
        file.write(" en plus")
        file.close()
    except:
        print("Erreur ecriture fichier")
 
#---- Lecture du fichier
#--------- Renvoie un message d'erreur si une action ne reussie pas
    try:
#--------- Ouverture du fichier en lecture seule
        file = open("/sd/data1.txt", "r")
#--------- Place le curseur avant la lecture
        #file.seek(17)
#--------- Enregistre le contenu du fichier dans la variable
        context = file.read()
        file.close()
        print(str(context))
    except:
        print("Erreur lecture")
        
    '''
    '''
#---------------// Commentaires Lecture/Ecriture SD//------------------------------------------------------------------------------------------------------------

    #---- Ecriture
        - Si on ferme le fichier et qu'on le reouvre on écrase les données
            -> Solution : Utiliser file.seek(xx) pour bouger le curseur jusqu'au bout du document puis ecrire le texte
            -> Remarque : on peut ecraser les donnees grace au placement du curseurs /!\ attention perte des données en aval du curseur

#----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''
    
    while True:
        '''
#---------------// Debug Bouton //-------------------------------------------------------------------------------------------------------------
#---- Debug avec bouton poussoir
#-------- Recuperation de la valeur du bouton
        button_level = bouton.value()
#-------- Si le bouton à change d'etat
        if button_level != old_button_level:
#-------- Affichage de l'etat du bouton
            print("Niveau logique du bouton : ", button_level)
#-------- Mise a jour de l'etat precedent
            old_button_level = button_level
#-------- Change l'état de la LED
            green.toggle()
#----------------------------------------------------------------------------------------------------------------------------------------------------------
        '''
        
        
#---------------// Debug UART //---------------------------------------------------------------------------------------------------------------
#---- Test pour faire la fonction toute les 1000ms
        if boucle % 1000 == 0:
#---- Affichage du port UART initialise
            #print("running...." + str(UART_ID) + "\n")
#---- Ecriture du message sur le bus serie
            green.toggle()
#---- Lecture du port de reception
        message_received = uart1.read()
#---- Si le buffer de reception n'est pas vide
        if not (message_received is None) :
#---- Affichage du temps puis du message recu apres decodage au format UTF-8
            print(str(time()) + " : Message reçu : \n" + message_received.decode("utf-8"))
#---- Gestion de la boucle temporelle
        utime.sleep_ms(1)
        boucle = boucle+1
#----------------------------------------------------------------------------------------------------------------------------------------------------------
        

        '''
#---------------// Debug GPS //------------------------------------------------------------------------------------------------------------------
#---- Initialisation de la variable timeout
        timeout = 0
#---- Tableau pour les valeurs de date et heure
        data_list = [0] * 16
#---- GPS non pret
        gps.gps_ok = False
#---- Recherche de signal GPS
#-------- Si le temps est depasse, led rouge clignote et message erreur
        while gps.gps_ok == False and timeout <= MAX_TIME:
#---- Encrement toute les secondes du compteur timeout
            timeout += 1
#---- Clignotement led verte (fonctionnement normal)
            green.toggle()
#---- Temporisation 1s
            await asyncio.sleep(1)  # synchronisation après une réception

#---- Recuperation des donnees si signal GPS
        if gps.gps_ok == True:
#---- Fonction de lecture des donnees et sauvegarde dans data_list
            gps.frame2bin(data_list)
#---- Affichage de l'heure GPS
            print("Heure : " + data_list[0] + ":" + data_list[1] + ":" + data_list[2])
#---- Affichage de la date GPS
            print("Date : " + data_list[5] + "/" + data_list[4] + "/" + data_list[3])
            print("------------------------------------------------------")
        else:
            if boucle % 250 == 0:
                red.toggle()
                print("Probleme GPS")
#---- 5 indique que le GPS n'a pas été reçu correctement
                data_list[1] = 5

        utime.sleep_ms(1)
        boucle = boucle+1
#----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''

    '''
#---------------// Debug RTC alarm //--------------------------------------------------------------------------------------------------------        
#---- Temporisation 1s.
        utime.sleep_ms(1000)
#---- Affichage temps execution en seconde    
        print("Temps : " + str(boucle))#---- Read secondes registerls
#---- Recuperation heure et date
        hour = rtc.read_clock()
        date = rtc.read_calendar()
#---- Affichage de la date et de l'heure
        #print("Date : " + str(date[0]) + "/" + str(date[1]) + "/" + str(date[2]))
        print("Date : " + str(hour[0]) + ":" + str(hour[1]) + ":" + str(hour[2]))
#---- Variable de comptage du timer
        boucle = boucle +1
#---- Variable verification registre alarme
        alarm_check = bytearray([0x91])
#---- Lecture registre alarme
        cs.high()
        rtcspi.write(alarm_check)
        rtcspi.readinto(alarm_check)
        cs.low()
#---- Etat alarme
        alarm_bit = (int.from_bytes(alarm_check, "big") >> 3) & 0x01
#---- Affichage de l'etat
        print("Alarm : " + str(alarm_bit))
        print("-------------------------------------------------------")
        #mkbus_pwr.off()
        #perif_pwr.off()
#---- Led temoin
        green.toggle()
#-----------------------------------------------------------------------------------------------------------------------------------------------------------
    '''
    
    #machine.deepsleep()
asyncio.run(main())

