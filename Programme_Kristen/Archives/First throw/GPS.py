import uasyncio as asyncio
from pyb import UART
import ustruct
#"$GNRMC,120527.00,A,10.38528,N,17957.57010,E,0.019,,010820,,,D*62\r\n"
class GPS:
    def __init__(self, uart_num):
        self.uart = UART(uart_num, 115200)
        self.sreader = asyncio.StreamReader(self.uart)
        self.frame_str = ""
        self.frame_list = []
        self.gps_ok = False
        asyncio.create_task(self.receiver())
        
    def read_num(self, vlist, inum):
        n = bytearray(2)
        n[0] = ord(vlist[inum * 2])-0x30
        n[1] = ord(vlist[inum * 2 + 1])-0x30
        return n[0] * 10 + n[1]
    
    def to_deg_dec(self, num):
        vint = num // 100
        vdec = num - vint * 100
        vdec = vdec / 60
        res = int((vint + vdec) * 100000)
        return res

    async def receiver(self):
        while True:
            res = await self.sreader.readline()
            self.gps_ok = False
            self.frame_str = res.decode("UTF8")
            print(str(len(self.frame_str)))
            if len(self.frame_str) > 40:
                self.frame_list = self.frame_str.split(',')
                if self.frame_list[2]=='A':
                    self.gps_ok = True
               
    def frame2bin(self, data_list):
        # Time information : HHMN
        data_list[0] = self.read_num(self.frame_list[1], 0) # Lecture de l'heure
        data_list[1] = self.read_num(self.frame_list[1], 1) # Lecture des minutes
        data_list[2] = self.read_num(self.frame_list[1], 2) # Lecture des secondes
        
        # Date information : YYMMDD
        data_list[3] = self.read_num(self.frame_list[9], 2)  # Année
        data_list[4] = self.read_num(self.frame_list[9], 1)  # Mois
        data_list[5] = self.read_num(self.frame_list[9], 0)  # Jour
        
        varray = bytearray(4)
        # Latitude
        val = self.to_deg_dec(float(self.frame_list[3]))
        varray = ustruct.pack(">l", val)
        data_list[6] = varray[1]
        data_list[7] = varray[2]
        data_list[8] = varray[3]
        data_list[9] = ord(self.frame_list[4])  # North ou South (code ASCII de 'N' ou 'S')
        
        # Longitude
        val = self.to_deg_dec(float(self.frame_list[5]))
        varray = ustruct.pack(">l", val)
        data_list[10] = varray[0]
        data_list[11] = varray[1]
        data_list[12] = varray[2]
        data_list[13] = varray[3]
        
        data_list[14] = ord(self.frame_list[6])  # East ou West(code ASCII de 'E' ou 'W')

#End of File

        
