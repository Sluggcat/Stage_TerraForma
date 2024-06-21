import pyb
import utime

class INA:
    CONFIG = 0x00
    SHUNT_V = 0x01
    BUS_V = 0x02
    POWER = 0x03
    CURRENT = 0x04
    CALIB = 0x05
    
    @staticmethod
    def bytes_to_int(msb, lsb):
        if not msb & 0x80:
            return msb << 8 | lsb
        return - (((msb ^ 255) << 8) | (lsb ^ 255) + 1)
    
    def __init__(self, i2c_num, address):
        self.i2c = pyb.I2C(i2c_num, pyb.I2C.MASTER)
        self.addr = address
        if not self.i2c.is_ready(self.addr):
            raise ValueError("No I2C device on I2C bus "+str(i2c_num)+' at address '+str(self.addr))
        buf = bytearray(2)
        buf[0] = 0x35
        buf[1] = 0x55
        self.i2c.mem_write(buf, self.addr, INA.CALIB)
        
    @property    
    def conf(self):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, INA.CONFIG)
        return buf
    
    @property
    def bus_v(self):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, INA.BUS_V)
        return INA.bytes_to_int(buf[0],buf[1])

    @property
    def shunt_v(self):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, INA.SHUNT_V)
        return INA.bytes_to_int(buf[0],buf[1])*10

    @property
    def voltage(self):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, INA.BUS_V)
        res = INA.bytes_to_int(buf[0], buf[1])
        volt = (res>>3) * 4
        return volt
        
    @property
    def current(self):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, INA.CURRENT)
        return INA.bytes_to_int(buf[0],buf[1]) * 30
    
    @property
    def power(self):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, INA.POWER)
        return INA.bytes_to_int(buf[0],buf[1])
    
    @property
    def calib(self):
        buf = bytearray(2)
        self.i2c.mem_read(buf, self.addr, INA.CALIB)
        return INA.bytes_to_int(buf[0],buf[1])        
