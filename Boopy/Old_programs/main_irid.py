'''
BOOPY V2 - version 4.6 du 231005:
Test en mer à Banyuls
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
from boopy_temp import boopy_temp10  # Capteur de température interne
from boopy_ptu import boopy_ptu   # Capteur de pression et humidité
from boopy_mpu import boopy_mpu  # Centrale inertielle
from boopy_extern import extern_temp10  # Capteurs externes
from boopy_energy import boopy_solar, boopy_battery, boopy_user
from iridium import IRID

def main_irid():
    main_pwr = Pin("MAIN_PWR", Pin.OUT_PP) 
    main_pwr.on()
    
    utime.sleep(1)
    
    rtc = RTC(5)  # RTC sur le SPI_5
    rtc.clear_alarm()
    rtc.set_RTC()    
    wakeup = 59
    rtc.set_alarm(1, 0, wakeup) # jour 1, 0h, wakeup_minutes
    
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
    n = 6
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

    #--- Lire le fichier numéro de trame et incrémenter ce numéro
    try:
        file = open("/flash/frame_cnt.txt", "r")
        iframe = int(file.readline())
        file.close()
    except:
        iframe = 0
    iframe += 1    

    print("Frame #"+str(iframe))
    
    data_list = [0] * 49 # Initialisation de la trame de mesures en octets
 
    #--- Attente de la première trame GPS en moins de 12 minutes
    gps = GPS(4)  # GPS sur l'UART4 
    timeout = 0
    gps.gps_ok = False
    while gps.gps_ok == False and timeout < 720:
        timeout += 1
        green.toggle()
        await asyncio.sleep(1)
    
    #--- Faire les acquisitions des capteurs (Pause de 30 secondes)
    if gps.gps_ok == True:
        green.on()
    else:
        green.off()
    
    red.on()
    # Température interne - 2 bytes
    temp_int = boopy_temp10()
    bytes_list = temp_int.to_bytes(2, 'big', True)
    data_list[14] = bytes_list[0]
    data_list[15] = bytes_list[1]
    red.toggle()

    # Température externe - 2 bytes
    temp_ext = extern_temp10()
    bytes_list = temp_ext.to_bytes(2, 'big', True)
    data_list[16] = bytes_list[0]
    data_list[17] = bytes_list[1]
    
    # Lumière - 3 bytes vide (ancien capteur retiré maintenant)
    data_list[18] = 0
    data_list[19] = 0 
    data_list[20] = 0
    red.toggle()

    # Pression  Humidité - 2 bytes x 2
    pr, hu = boopy_ptu()
    bytes_list = pr.to_bytes(2, 'big', True)
    data_list[21] = bytes_list[0]
    data_list[22] = bytes_list[1] 
    bytes_list = hu.to_bytes(2, 'big', True)
    data_list[23] = bytes_list[0]
    data_list[24] = bytes_list[1]
    red.toggle()

    # Centrale inertielle - 1 byte x 6
    (ax_max, ay_max, az_max, gx_max, gy_max, gz_max) = boopy_mpu()
    data_list[25] = ax_max
    data_list[26] = ay_max
    data_list[27] = az_max
    data_list[28] = gx_max
    data_list[29] = gy_max
    data_list[30] = gz_max     
    red.toggle()

    # Solaire
    current = 0
    n = 10
    while current == 0 and n > 0:
        volt, current = boopy_solar()
        n -= 1
    
    bytes_list = volt.to_bytes(2, 'big', True)
    data_list[31] = bytes_list[0]
    data_list[32] = bytes_list[1]
    
    sig = '+'
    if current < 0:
        current = - current
        sig = '-'
    bytes_list = current.to_bytes(2, 'big', True)
    data_list[33] = ord(sig)
    data_list[34] = bytes_list[0]
    data_list[35] = bytes_list[1]
    red.toggle()

    # Batterie
    current = 0
    n = 10
    while current == 0 and n > 0:
        volt, current = boopy_battery()
        n -= 1
    
    bytes_list = volt.to_bytes(2, 'big', True)
    data_list[36] = bytes_list[0]
    data_list[37] = bytes_list[1]
    
    sig = '+'
    if current < 0:
        current = - current
        sig = '-'
    bytes_list = current.to_bytes(2, 'big', True)
    data_list[38] = ord(sig)
    data_list[39] = bytes_list[0]
    data_list[40] = bytes_list[1]
    red.toggle()

    # Utilisateur
    current = 0
    n = 10
    while current == 0 and n > 0:        
        volt, current = boopy_user()
        n -= 1

    bytes_list = volt.to_bytes(2, 'big', True)
    data_list[41] = bytes_list[0]
    data_list[42] = bytes_list[1]
    
    sig = '+'
    if current < 0:
        current = - current
        sig = '-'
    bytes_list = current.to_bytes(2, 'big', True)
    data_list[43] = ord(sig)
    data_list[44] = bytes_list[0]
    data_list[45] = bytes_list[1]
    red.toggle()

    # Status chargeur --------------------------------------------
    iread = 0
    chg_on = 0
    while iread < 100:  #--- Lecture pendant 1 sec
        if chrg_act.value() == 0:
            chg_on += 1
        iread += 1
        utime.sleep_ms(10)  
        
    #--- Interprétation du résultat    
    if chg_on > 0:
        if chg_on == 100:
            status = 0x0F
        else:
            status = 0x03
    else:
        status = 0x00

    if chrg_done.value() == 0:
        status = 0xFF
       
    data_list[46] = status
    red.off()

    #--- Attente d'une nouvelle trame GPS pour data_list en moins de 2 minutes 
    timeout = 0
    gps.gps_ok = False
    while gps.gps_ok == False and timeout < 240:
        timeout += 1
        green.toggle()
        await asyncio.sleep(1) # synchronisation après une réception     

    #--- Si la trame GPS est valide, la transformer en octets intégrés dans la trame de données générale
    if gps.gps_ok == True:
        gps.frame2bin(data_list)
        green.on()
    else:
        green.off()
        data_list[1] = 5 

    #--- Calcul de la Checksum
    cs = 0
    for i in data_list:
        cs +=i
    bytes_list = cs.to_bytes(2, 'big', True)
    data_list[47] = bytes_list[0]
    data_list[48] = bytes_list[1]        

    # Ecriture de la trame dans un fichier
    filename = "/flash/data" + str(iframe) + ".txt"
    try:
        file = open(filename, "w")
        try:
            file.write(",".join(str(i) for i in data_list))
        finally:
            file.close()
    except:
        error = 1

    #---- Sauvegarde sur carte uSD
        
    if SDCard().present():
        sd_error = 0
        try:
            uos.mount(SDCard(), "/sd")
        except:
            sd_error = 1       
        try:
            file = open("/sd/data_oct23.csv", 'a')
            try:
                file.write(",".join(str(i) for i in data_list))
                file.write("\r\n")
            finally:
                file.close()
        except:
            sd_error = 2
    irid_tries = 0        
    #--------------------------------------------------------------
    #--- Test si fin de cycle pour envoyer les données sur IRIDIUM
    if iframe > 5:
        ext_sensor_pwr.off()
        utime.sleep(0.1)
        int_sensor_pwr.off()
        utime.sleep(0.1)
        perif_pwr.off()
        utime.sleep(0.1)

        iridium_pwr.on()
        utime.sleep(30)
        
        try:
            irid = IRID()
        except:
            error = 1
   
        #--- Construction du paquet de 6 trames
        packet_str = ""
        data_packet = [0] * 294  # 6 * 49 octets
        ifile = 0
        while ifile < iframe:
            ifile+=1
            filename = "/flash/data" + str(ifile) + ".txt"
            try:
                file = open(filename, "r") 
                packet_str += file.readline() + ','
                file.close()
            except:
                error = 1
        
        data_packet = packet_str.split(",") # Transforme le texte en liste de texte nombre ; '112','14'        
        del data_packet[-1] # retire le '' placé en dernière position de la liste data_frame produit par la dernière ','

        # Placer le packet de 6 trames dans le buffer d'envoi du modem
        res = await irid.send("AT+SBDWB=294", 4)
        res = await irid.send_bin(data_packet, 4)

        # Réaliser au maximum 15 tentatives d'envoi de data
        print("start sequence\r")
        ntry = 15
        another_try = True
        while another_try == True and ntry > 0:
            blue.toggle()
            irid.count = 20 # Attendre la réponse du modem en secondes
            print("Tentatives #" + str(16-ntry) + "\r")
            res = await irid.send("AT+SBDIX", 25)
            if res:
                lstatus = res[1].decode("utf8")
                ilist = lstatus.split("+SBDIX:")
                stlist = ilist[1].split(',')
                n = int(stlist[0]) # Récupère le compte-rendu d'envoi Iridium
                if n > 4:
                    utime.sleep(30) # Attendre 30 secondes avant la prochaine tentative
                    ntry-=1
                else:
                    another_try = False
            else:
                ntry-=1

        print("Modem OFF")
        irid.uart_off()
        iridium_pwr.off()
        utime.sleep(240) # Pour éviter d'avoir des retours electriques par l'UART : attendre la décharge du condo du modem avant de partir en sommeil
        iframe = 0
        irid_tries = 16 - ntry
    # Ecrit dans le fichier le numéro de la trame
    try:
        file = open("/flash/frame_cnt.txt", "w")
        file.write(str(iframe) + '\n')
        file.close()
    except:
        iframe = 0

    perif_pwr.on()
    utime.sleep(0.1)
    int_sensor_pwr.on()
    utime.sleep(0.1)

    #--- Attente d'une troisième trame GPS pour le sommeil en moins de 2 minutes
    print("   - Third GPS sequence...")
    timeout = 0
    gps.gps_ok = False
    minutes = 0
    while ((gps.gps_ok == False) or (minutes < 1)) and ( timeout < 240):
        timeout += 1
        green.toggle()
        if gps.gps_ok == True:
            gps.frame2bin(data_list)
            minutes = data_list[1]
        print("Minutes : " + str(minutes))
        await asyncio.sleep(1)    

    # Calcul du temps de sommeil en minutes
    minutes = data_list[1]
    if minutes > 0:
        wakeup = int(60 - minutes)
    else:
        wakeup = 59

    #--- Programmation de l'alarme de reveil de la RTC
    #wakeup = 3
    rtc.clear_alarm()
    rtc.set_RTC()
    rtc.set_alarm(1, 0, wakeup) # jour 1, 0h, wakeup_minutes

    try:
        file = open("/sd/debug.txt", "a")
        file.write(str(data_list[0]) + ':' + str(data_list[1]) + ' ')
        file.write(str(data_list[2]) + '/' + str(data_list[3]) + '/' + str(data_list[4]) + ',')
        file.write(str(wakeup) + ',' + str(irid_tries) + '\n')
        file.close()
    except:
        print("Erreur ecriture debug")
        
    utime.sleep(2)  # Attente de 2 secondes avant de...couper l'alim
    
    while(1):
        int_sensor_pwr.off()
        perif_pwr.off()
        main_pwr.off()
        utime.sleep(0.2)
        blue.toggle()
    

#End of File