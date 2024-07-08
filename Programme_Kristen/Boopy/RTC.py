'''
Bibliotheque de gestion de la RTC RV-2123-C2
Configurée pour programmer un réveil de la carte toutes les heures

  Registers   Sec  Min  Hours Days  Weekdays  Months Years
              0x02 0x03 0x04  0x05  0x06      0x07   0x08

Le premier octet envoyé est la commande :
   Bit 7 -  Ecriture (1) ou lecture (0)
   Bits 6,5,4 - Toujours '001'
   Bits 3,2,1,0 - Adresse du registre
'''
import pyb
import ustruct

class RTC:
    def reset(self):
        cmd = bytearray(2)
        cmd[0] = 0x10  # bit7=0 : écriture - bits0-3=1 : à partir l'@ 0x00 (Control_1)
        cmd[1] = 0x58  # set RESET bits in Control_1 register
        self.cs.high()
        self.spi.send(cmd)
        self.cs.low()
        
    def __init__(self, spi_num):
        self.cs = pyb.Pin(pyb.Pin.cpu.F6, pyb.Pin.OUT_PP)
        self.cs.low()
        self.spi = pyb.SPI(spi_num, pyb.SPI.MASTER, baudrate=600000, polarity=0, phase=1, crc=None)
        self.reset()

    def int_to_bcd(self, num):
        res = (num // 10) * 16 + (num % 10)
        return res
    
    def clear_alarm(self):
        cmd = bytearray(2)
        cmd[0] = 0x11  # bit7=0 : écriture - bits0-3=1 : à partir l'@ 0x01 (Control_2)
        cmd[1] = 0x02  # set AIE high clear AF
        self.cs.high()
        self.spi.send(cmd)
        self.cs.low()

    def set_alarm(self, dd, hh, mn):
        cmd = bytearray(5)
        cmd[0] = 0x19  # bit7=0 : écriture - bits0-3=9 : à partir l'@ 0x09 (Alarm minute)
        cmd[1] = self.int_to_bcd(mn)  # Minutes alarm reg:0x09
        cmd[2] = self.int_to_bcd(hh)  # Hours alarm reg:0x0A
        cmd[3] = self.int_to_bcd(dd)  # Days 28th alarm reg:0x0B
        cmd[4] = 0x00  # WeekofDay alarm #0=sunday reg:0x0C
        self.cs.high()
        self.spi.send(cmd)
        self.cs.low()
        
    def set_RTC(self):
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
        self.cs.high()
        self.spi.send(cmd)
        self.cs.low()
        
    def set_clock(self, hh, mn, ss):
        cmd = bytearray(4)
        cmd[0] = 0x12  # bit7=0 : écriture - bits0-3=2 : à partir de l'@ 0x02 (secondes)
        cmd[1] = self.int_to_bcd(ss)  # 50 seconds reg:0x02
        cmd[2] = self.int_to_bcd(mn)  # minutes reg:0x03
        cmd[3] = self.int_to_bcd(hh) # heures reg:0x04
        self.cs.high()
        self.spi.send(cmd)
        self.cs.low()

    def set_calendar(self, yy, mm, dd):
        cmd = bytearray(5)
        cmd[0] = 0x15  # bit7=0 : écriture - bits0-3=5 : à partir l'@ 0x05 (days)
        cmd[1] = self.int_to_bcd(dd)  # Day reg:0x05
        cmd[2] = 0x00  # WeekofDay #0=sunday reg:0x06
        cmd[3] = self.int_to_bcd(mm)  # Month reg:0x07
        cmd[4] = self.int_to_bcd(yy)  # Year reg:0x08
        self.cs.high()
        self.spi.send(cmd)
        self.cs.low()

    def read_clock(self):
        cmd = bytearray(1)
        res = bytearray(3)
        cmd[0] = 0x92  # bit7=1 : lecture - bits0-3=2 : à partir de l'@ 0x02 ()
        self.cs.high()
        self.spi.send(cmd)
        self.spi.readinto(res)
        self.cs.low()
        return(res[2], res[1], res[0])  # Heure, Minute, Seconde

    def read_calendar(self):
        cmd = bytearray(1)
        res = bytearray(4)
        cmd[0] = 0x95  # bit7=1 : lecture - bits0-3=5 : à partir de l'@ 0x05 (Days)
        self.cs.high()
        self.spi.send(cmd)
        self.spi.readinto(res)
        self.cs.low()
        return(res[0], res[2], res[3])  # Jours, Mois, Année
    
    
