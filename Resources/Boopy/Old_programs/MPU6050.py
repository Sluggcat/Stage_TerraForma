import pyb
import utime

class MPU:
    GYR_CONF = 0x1B
    ACC_CONF = 0x1C
    RAW_DATA = 0x3B
    PWR_MGMT_1 = 0x6B
    WHO_AM_I = 0x75

    @staticmethod
    def bytes_to_int(msb, lsb):
        if not msb & 0x80:
            return msb << 8 | lsb
        return - (((msb ^ 255) << 8) | (lsb ^ 255) + 1)

    @staticmethod
    def bytes_to_abs(msb, lsb):
        if not msb & 0x80:
            return msb << 8 | lsb
        return (((msb ^ 255) << 8) | (lsb ^ 255) + 1)

    def __init__(self, i2c_num, address = 0x68) :
        buf = bytearray(1)
        self.i2c = pyb.I2C(i2c_num, pyb.I2C.MASTER)
        self.addr = address
        if not self.i2c.is_ready(self.addr) :
            raise ValueError('No I2C device on I2C bus ' + str(i2c_num) + ' at address ' + str(self.addr))
        buf[0] = 0x80
        self.i2c.mem_write(buf, self.addr, MPU.PWR_MGMT_1)
        utime.sleep_ms(100)
        buf[0] = 0x01
        self.i2c.mem_write(buf, self.addr, MPU.PWR_MGMT_1)
        self.acc_sens = 16384
        self.gyr_sens = 1310
        self.set_acc_conf(8)
        self.set_gyr_conf(50)

    def get_data(self):
        buf = bytearray(14)
        try:
          self.i2c.mem_read(buf, self.addr, MPU.RAW_DATA)
        except:
            ax = 0
            ay = 0
            az = 0
            tp = 0
            gx = 0
            gy = 0
            gz = 0
        else:
            ax = int(MPU.bytes_to_abs(buf[0], buf[1]) / self.acc_sens * 10)
            ay = int(MPU.bytes_to_abs(buf[2], buf[3]) / self.acc_sens * 10)
            az = int(MPU.bytes_to_abs(buf[4], buf[5]) / self.acc_sens * 10)
            tp = int((MPU.bytes_to_int(buf[6], buf[7]) / 340 + 36.53) * 10)
            gx = int(MPU.bytes_to_abs(buf[8], buf[9]) * 10 / self.gyr_sens)
            gy = int(MPU.bytes_to_abs(buf[10], buf[11]) * 10 / self.gyr_sens)
            gz = int(MPU.bytes_to_abs(buf[12], buf[13]) * 10 / self.gyr_sens)
        return(ax, ay, az, tp, gx, gy, gz)

    def get_dataS(self):
        buf = bytearray(14)
        try:
          self.i2c.mem_read(buf, self.addr, MPU.RAW_DATA)
        except:
            ax = 0
            ay = 0
            az = 0
            tp = 0
            gx = 0
            gy = 0
            gz = 0
        else:
            ax = int(MPU.bytes_to_int(buf[0], buf[1]) / self.acc_sens * 100)
            ay = int(MPU.bytes_to_int(buf[2], buf[3]) / self.acc_sens * 100)
            az = int(MPU.bytes_to_int(buf[4], buf[5]) / self.acc_sens * 100)
            tp = int((MPU.bytes_to_int(buf[6], buf[7]) / 340 + 36.53) * 10)
            gx = int(MPU.bytes_to_int(buf[8], buf[9]) * 10 / self.gyr_sens)
            gy = int(MPU.bytes_to_int(buf[10], buf[11]) * 10 / self.gyr_sens)
            gz = int(MPU.bytes_to_int(buf[12], buf[13]) * 10 / self.gyr_sens)
        return(ax, ay, az, tp, gx, gy, gz)

    def get_acc_text( self, data_list, index_x, index_y, index_z ):
        buf = bytearray(14)
        try:
          self.i2c.mem_read(buf, self.addr, MPU.RAW_DATA)
        except:
            ax = 0
            ay = 0
            az = 0
        else:
            ax = MPU.bytes_to_abs(buf[0], buf[1]) / self.acc_sens
            ay = MPU.bytes_to_abs(buf[2], buf[3]) / self.acc_sens
            az = MPU.bytes_to_abs(buf[4], buf[5]) / self.acc_sens
            ax_str = str( "%.1f" % ax )
            ay_str = str( "%.1f" % ay )
            az_str = str( "%.1f" % az )
            data_list[index_x] = ax_str.replace('.', ',')
            data_list[index_y] = ay_str.replace('.', ',')
            data_list[index_z] = az_str.replace('.', ',')            
    
    def get_gyro_text( self, data_list, index_x, index_y, index_z ):
        buf = bytearray(14)
        try:
          self.i2c.mem_read(buf, self.addr, MPU.RAW_DATA)
        except:
            gx = 0
            gy = 0
            gz = 0
        else:
            gx = int(MPU.bytes_to_abs(buf[8], buf[9]) * 10 / self.gyr_sens) - 4
            gy = int(MPU.bytes_to_abs(buf[10], buf[11]) * 10 / self.gyr_sens) - 1
            gz = int(MPU.bytes_to_abs(buf[12], buf[13]) * 10 / self.gyr_sens)
            gx_str = str( "%.1f" % gx )
            gy_str = str( "%.1f" % gy )
            gz_str = str( "%.1f" % gz )
            data_list[index_x] = gx_str.replace('.', ',')
            data_list[index_y] = gy_str.replace('.', ',')
            data_list[index_z] = gz_str.replace('.', ',')
            
    def set_acc_conf(self, acc_range):
        buf = bytearray(1)
        if acc_range==2:
            buf[0]=0
            self.acc_sens = 16384
        elif acc_range==4:
            buf[0]=8
            self.acc_sens = 8192
        elif acc_range==8:
            buf[0]=16
            self.acc_sens = 4096
        elif acc_range==16:
            buf[0]=24
            self.acc_sens = 2048
        else:
            buf[0]=24
            self.acc_sens = 2048
        self.i2c.mem_write(buf, self.addr, MPU.ACC_CONF)

    def set_gyr_conf(self, gyr_range):
        buf = bytearray(1)
        if gyr_range==25:
            buf[0]=0
            self.gyr_sens = 1310
        elif gyr_range==50:
            buf[0]=8
            self.gyr_sens = 655
        elif gyr_range==100:
            buf[0]=16
            self.gyr_sens = 328
        elif gyr_range==200:
            buf[0]=24
            self.gyr_sens = 164
        else:
            buf[0]=24
            self.gyr_sens = 164
        self.i2c.mem_write(buf, self.addr, MPU.GYR_CONF) 
