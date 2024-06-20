# MCP9808 temperature sensor library for MicroPython on Pyboard or STM32 based boards.
# Datasheet link : http://ww1.microchip.com/downloads/en/DeviceDoc/MCP9808-0.5C-Maximum-Accuracy-Digital-Temperature-Sensor-Data-Sheet-DS20005095B.pdf

'''
Temperature est contenue dans 2 registres de 8 bits
Premier registre
  7    6    5    4    3    2    1    0
               sign  2^7  2^6  2^5  2^4
Second registre 
  7    6    5    4    3    2    1    0
 2^3  2^2  2^1  2^0  2^-1 2^-2 2^-3 2^-4
'''

'''
Mesure de température
Gamme : -10°C à +50°C
Codage : -10.0°C -> 0_(0x0000)
         +100.0°C -> 1000_(0x03E8)
Résolution : 10ième de degré codé sur 2 octets
'''
import pyb
    
class MCP:
    TEMPER = 0x05
    MAN_ID = 0x06
    DEV_ID = 0x07
    RESOLU = 0x08
    
    RES_MIN = const(0)
    RES_LOW = const(1)
    RES_HIG = const(2)
    RES_MAX = const(3)    
    
    def __init__(self, i2c_num, address = 0x18):
        self.i2c = pyb.I2C(i2c_num, pyb.I2C.MASTER)
        self.addr = address
        if not self.i2c.is_ready(self.addr) :
            raise ValueError('No I2C device on I2C bus ' + str(i2c_num) + ' at address ' + str(self.addr))

    def get_temp_str( self ):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, MCP.TEMPER)
        h = (buf[0] & 0x0F) << 4
        dec = ((buf[1] & 0x0F) * 100) >> 4
        l = buf[1] >> 4
        data_str = ''
        if buf[0] & 0x10 == 0x10:
            temp = 256 - (h + l)
            tdec = 256 - dec
            data_str = '-'
        else:
            temp = h + l
            tdec = dec
        temp = temp + tdec / 100    
        data_str = data_str + str( "%.2f" % temp )
        data_str = data_str.replace('.', ',')
        return(data_str)
    
    def get_temp_text( self, data_list, index ):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, MCP.TEMPER)
        h = (buf[0] & 0x0F) << 4
        dec = ((buf[1] & 0x0F) * 100) >> 4
        l = buf[1] >> 4
        data_str = ''
        if buf[0] & 0x10 == 0x10:
            temp = 256 - (h + l)
            tdec = 256 - dec
            data_str = '-'
        else:
            temp = h + l
            tdec = dec
        temp = temp + tdec / 100    
        data_str = data_str + str( "%.2f" % temp )
        data_list[index] = data_str.replace('.', ',')
        
    def get_temp(self):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, MCP.TEMPER)
        h = (buf[0] & 0x0F) << 4
        dec = ((buf[1] & 0x0F) * 100) >> 4
        l = buf[1] >> 4
        if buf[0] & 0x10 == 0x10:
            temp = 256 - (h + l)
            tdec = 256 - dec
        else:
            temp = h + l
            tdec = dec
        return(temp, dec)    


    def set_res(self, res):
        if res in [RES_MIN, RES_LOW, RES_HIG, RES_MAX]:
            buf = bytearray(1)
            buf[0] = res
            self.i2c.mem_write(buf, self.addr, MCP.RESOLU)
        else:
            print("parameter error")
            
    def get_res(self):
        buf = bytearray(1)
        self.i2c.mem_read(buf, self.addr, MCP.RESOLU)
        print(hex(buf[0]))        
        
    def man_id(self):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, MCP.MAN_ID)
        print(hex(buf[0]), hex(buf[1]))
        
    def dev_id(self):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, MCP.DEV_ID)
        print(hex(buf[0]), hex(buf[1]))
        
#end of file        