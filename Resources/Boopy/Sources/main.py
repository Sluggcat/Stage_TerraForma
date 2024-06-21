"""
BOOPY - version 2.1 du 210823:
Corrections :
    - commande alimentation
    - conditions de reset
    - alimentation connecteur µSD
    - programmation RTC
"""
import utime
import ustruct
import uasyncio as asyncio
from pyb import SDCard
from pyb import Pin
from GPS import GPS
from RTC import RTC
from pyb import UART
from pyb import LED
from boopy_temp import boopy_temp10  # Capteur de température interne
from boopy_light import boopy_light  # Capteur de lumière
from boopy_ptu import boopy_ptu  # Capteur de pression et humidité
from boopy_mpu import boopy_mpu  # Centrale inertielle
from boopy_extern import extern_temp10  # Capteurs externes (température)
from boopy_energy import boopy_solar, boopy_battery, boopy_user
from iridium import IRID


async def main():
    # -1--- Sequence d'initialisation
    # -1.1--- Vérouillage de la tension de la carte
    main_pwr = Pin("MAIN_PWR", Pin.OUT_PP)
    main_pwr.on()

    utime.sleep(1)

    # -1.2--- Programmation de l'alarme de reveil de la RTC
    rtc = RTC(5)  # RTC sur le SPI_5
    rtc.clear_alarm()
    rtc.set_RTC()
    wakeup = 59
    rtc.set_alarm(1, 0, wakeup)  # jour 1, 0h, wakeup_minutes

    # -1.3--- Alimentation des périphériques dont le GPS
    perif_pwr = Pin("PERIF_PWR", Pin.OUT_PP)
    perif_pwr.on()

    utime.sleep(1)  # Tempo pour stabilisation des alimentations

    # -1.4--- Alimentation des capteurs internes
    int_sensor_pwr = Pin("INT_SENSOR_PWR", Pin.OUT_PP)
    int_sensor_pwr.on()

    # -1.5--- Alimentation des capteurs externes
    ext_sensor_pwr = Pin("EXT_SENSOR_PWR", Pin.OUT_PP)
    ext_sensor_pwr.off()

    # -1.6--- Alimentation du modem Iridium (off)
    iridium_pwr = Pin("IRIDIUM_PWR", Pin.OUT_PP)
    iridium_pwr.off()

    utime.sleep(1)  # Tempo pour stabilisation des alimentations

    # -1.7--- Configuration des GPIO (Buzzer et infos du chargeur)
    buz = Pin(Pin.cpu.G1, Pin.OUT_PP)
    buz.off()
    chrg_act = Pin("CHRG_ACT", Pin.IN)
    chrg_done = Pin("CHRG_DONE", Pin.IN)

    # -1.8--- Initialisation de la trame de mesures en octets
    data_list = [0] * 49

    # -1.9--- Séquence sonore et lumineuse
    red = LED(1)
    green = LED(2)
    blue = LED(3)
    n = 6
    while n > 0:
        blue.toggle()
        green.toggle()
        red.toggle()
        if buz.value() == 0:
            buz.on()
        else:
            buz.off()
        n -= 1
        utime.sleep(1)

    # -2--- Attente de la première trame GPS en moins de 2 minutes
    gps = GPS(4)  # GPS sur l'UART4

    timeout = 0
    gps.gps_ok = False
    while gps.gps_ok == False and timeout < 120:
        timeout += 1
        green.toggle()
        await asyncio.sleep(1)

    if timeout < 120:  # Si au bout de 2mn, pas de GPS, eteindre Led verte
        green.on()
    else:
        green.off()

    # -3--- Mesure des capteurs
    # Les résultats sont placés dans la trame de données : data_list
    red.on()  # Indique début acquisition capteurs
    # -3.1--- Température interne - 2 bytes
    temp = boopy_temp10()
    bytes_list = temp.to_bytes(2, "big", True)
    data_list[14] = bytes_list[0]
    data_list[15] = bytes_list[1]
    red.toggle()
    """
    #-3.2--- Température externe - 2 bytes (Non installé)
    temp = extern_temp10()
    bytes_list = temp.to_bytes(2, 'big', True)
    data_list[16] = bytes_list[0]
    data_list[17] = bytes_list[1]
    """

    # -3.3--- Lumière - 3 bytes
    lux = boopy_light()
    bytes_list = lux.to_bytes(3, "big", True)
    data_list[18] = bytes_list[0]
    data_list[19] = bytes_list[1]
    data_list[20] = bytes_list[2]
    red.toggle()

    # -3.4--- Pression  Humidité - 2 bytes x 2
    pr, hu = boopy_ptu()
    bytes_list = pr.to_bytes(2, "big", True)
    data_list[21] = bytes_list[0]
    data_list[22] = bytes_list[1]
    bytes_list = hu.to_bytes(2, "big", True)
    data_list[23] = bytes_list[0]
    data_list[24] = bytes_list[1]
    red.toggle()

    # -3.5--- Centrale inertielle - 1 byte x 6
    (ax_max, ay_max, az_max, gx_max, gy_max, gz_max) = boopy_mpu()
    data_list[25] = ax_max
    data_list[26] = ay_max
    data_list[27] = az_max
    data_list[28] = gx_max
    data_list[29] = gy_max
    data_list[30] = gz_max
    red.toggle()

    # -3.6--- Panneau Solaire
    current = 0
    n = 10
    while current == 0 and n > 0:
        volt, current = boopy_solar()
        n -= 1

    bytes_list = volt.to_bytes(2, "big", True)
    data_list[31] = bytes_list[0]
    data_list[32] = bytes_list[1]

    sig = "+"
    if current < 0:
        current = -current
        sig = "-"
    bytes_list = current.to_bytes(2, "big", True)
    data_list[33] = bytes_list[0]
    data_list[34] = bytes_list[1]
    data_list[35] = ord(sig)
    red.toggle()

    # -3.7--- Batterie
    current = 0
    n = 10
    while current == 0 and n > 0:
        volt, current = boopy_battery()
        n -= 1

    bytes_list = volt.to_bytes(2, "big", True)
    data_list[36] = bytes_list[0]
    data_list[37] = bytes_list[1]

    sig = "+"
    if current < 0:
        current = -current
        sig = "-"
    bytes_list = current.to_bytes(2, "big", True)
    data_list[38] = bytes_list[0]
    data_list[39] = bytes_list[1]
    data_list[40] = ord(sig)
    red.toggle()

    # -3.8--- Puissance utilisateur
    current = 0
    n = 10
    while current == 0 and n > 0:
        volt, current = boopy_user()
        n -= 1

    bytes_list = volt.to_bytes(2, "big", True)
    data_list[41] = bytes_list[0]
    data_list[42] = bytes_list[1]

    sig = "+"
    if current < 0:
        current = -current
        sig = "-"
    bytes_list = current.to_bytes(2, "big", True)
    data_list[43] = bytes_list[0]
    data_list[44] = bytes_list[1]
    data_list[45] = ord(sig)
    red.toggle()

    # -3.9--- Status chargeur
    iread = 0
    chg_on = 0
    while iread < 100:  # Lecture pendant 1 sec
        if chrg_act.value() == 0:
            chg_on += 1
        iread += 1
        utime.sleep_ms(10)

    # --3.10--- Interprétation des status du chargeur
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

    red.off()  # Fin des mesures des capteurs

    # -4--- Attente d'une nouvelle trame GPS pour data_list en moins de 2 minutes
    timeout = 0
    gps.gps_ok = False
    while gps.gps_ok == False and timeout < 120:
        timeout += 1
        green.toggle()
        await asyncio.sleep(1)  # synchronisation après une réception

    # -5--- Si la trame GPS est valide, la transformer en octets intégrés dans la trame de données générale
    if gps.gps_ok == True:
        gps.frame2bin(data_list)
        buz.on()
        utime.sleep(0.5)
        buz.off()
        green.on()
    else:
        green.off()
        data_list[1] = 5  # 5 indique que le GPS n'a pas été reçu correctement

    # -6--- Sauver la trame de données
    # -6.1--- Calcul de la Checksum
    cs = 0
    for i in data_list:
        cs += i
    bytes_list = cs.to_bytes(2, "big", True)
    data_list[47] = bytes_list[0]
    data_list[48] = bytes_list[1]

    # -6.2--- Lire le fichier numéro de trame, incrémenter ce numéro
    try:
        file = open("/flash/frame_cnt.txt", "r")
        iframe = int(file.readline())
        file.close()
    except:
        iframe = 0  # Par défaut, trame #0

    iframe += 1  # Nouvelle trame

    # -6.3--- Ecriture de la trame dans un fichier
    filename = "/flash/data" + str(iframe) + ".txt"
    try:
        file = open(filename, "w")
        try:
            file.write(",".join(str(i) for i in data_list))
        finally:
            file.close()
    except:
        error = 1

    # -7--- Test si fin de cycle pour envoyer les données sur IRIDIUM
    if iframe == 6:
        # -7.1--- Alimenter le modem Iridium
        iridium_pwr.on()
        utime.sleep(30)

        try:
            irid = IRID()  # Création de l'objet lié au modem Iridium
        except:
            error = 1
        # -7.2--- Lire des 6 dernières trames de données
        ifile = 0
        frame_str = ""
        data_frame = [0] * 294  # 6 * 49 octets : les 6 trames à envoyer

        while ifile < iframe:  # Lecture des 6 trames
            ifile += 1
            filename = "/flash/data" + str(ifile) + ".txt"
            try:
                file = open(filename, "r")
                frame_str += file.readline() + ","
                file.close()
            except:
                error = 1

        data_frame = frame_str.split(
            ","
        )  # Transforme le texte en liste de texte nombre ; '112','14'
        del data_frame[
            -1
        ]  # retire le '' placé en dernière position de la liste data_frame produit par la dernière ','

        # -7.3--- Placer les 6 trames dans le buffer d'envoi du modem
        irid.count = 4  # Attendre pendant 4 secondes la réponse du modem
        res = await irid.send("AT+SBDWB=294")

        irid.count = 4  # Attendre pendant 2 secondes la réponse du modem
        res = await irid.send_bin(data_frame)

        # -7.4--- Réaliser 15 tentatives d'envoi de data
        ntry = 15
        another_try = True
        while another_try == True and ntry > 0:
            blue.toggle()
            irid.count = 20  # Attendre pendant 20 secondes la réponse du modem
            res = await irid.send("AT+SBDIX")

            if res:
                lstatus = res[1].decode("utf8")
                ilist = lstatus.split("+SBDIX:")
                stlist = ilist[1].split(",")
                n = int(stlist[0])
                if n > 4:
                    utime.sleep(30)  # Attendre 30 secondes avant la prochaine tentative
                    ntry -= 1
                else:
                    another_try = False
            else:
                ntry -= 1
        # -7.5--- Couper le model et mettre à zéro l'identifiant des trames
        irid.uart_off()
        iridium_pwr.off()
        utime.sleep(
            240
        )  # Pour éviter d'avoir des retours electriques par l'UART : attendre la décharge du condo du modem avant de partir en sommeil
        iframe = 0  # Redémarrer un nouveau paquet de 6 trames

    # -8--- Ecrit dans le fichier le numéro de la trame
    try:
        file = open("/flash/frame_cnt.txt", "w")
        file.write(str(iframe) + "\n")
        file.close()
    except:
        iframe = 0

    # -8.1--- Attendre une minute sans rien faire pour être sûr que l'on dépasse bien le changement d'heure
    utime.sleep(60)

    # -9--- Attente d'une troisième trame GPS pour le sommeil en moins de 2 minutes
    timeout = 0
    gps.gps_ok = False
    while gps.gps_ok == False and timeout < 120:
        timeout += 1
        green.toggle()
        await asyncio.sleep(1)  # synchronisation après une réception

    # --- Si la trame GPS est valide, la transformer en octets intégrés dans la trame de données générale
    if gps.gps_ok == True:
        gps.frame2bin(data_list)
        green.on()
    else:
        green.off()
        data_list[1] = 5

    # -10--- Calcul du temps de sommeil en minutes
    minutes = data_list[1]
    if minutes > 0:
        wakeup = int(60 - minutes)
    else:
        wakeup = 59

    # -10.1--- Programmation de l'alarme de reveil de la RTC
    rtc.clear_alarm()
    rtc.set_RTC()
    rtc.set_alarm(1, 0, wakeup)  # jour 1, 0h, wakeup_minutes

    utime.sleep(5)  # Attente de 5 secondes avant de...couper l'alim

    # -11--- Couper l'alimentation
    while 1:
        int_sensor_pwr.off()
        perif_pwr.off()
        main_pwr.off()
        utime.sleep(0.2)
        red.toggle()


asyncio.run(main())

# End of File
