import pyb
import sys
import utime

class BME :
  DIG_T1 = 0x88
  DIG_T2 = 0x8A
  DIG_T3 = 0x8C 
  DIG_P1 = 0x8E
  DIG_P2 = 0x90
  DIG_P3 = 0x92
  DIG_P4 = 0x94
  DIG_P5 = 0x96
  DIG_P6 = 0x98
  DIG_P7 = 0x9A
  DIG_P8 = 0x9C
  DIG_P9 = 0x9E
  DIG_H1 = 0xA1
  DIG_H2 = 0xE1
  DIG_H3 = 0xE3
  DIG_H4 = 0xE4
  DIG_H5 = 0xE5
  DIG_H6 = 0xE7
  
  ID_REG = 0xD0
  HUM_CTRL_REG = 0xF2
  STAT_REG = 0xF3
  MEAS_REG = 0xF4
  CONF_REG = 0xF5
  PRESS_REG = 0xF7
  TEMP_REG = 0xFA
  HUM_REG = 0xFD

  MEASURING = 0b1000
  UPDATING  = 0b0001

  SKIP = 0b000
  OVRSAMP_1 = 0b001
  OVRSAMP_2 = 0b010
  OVRSAMP_4 = 0b011
  OVRSAMP_8 = 0b100
  OVRSAMP_16 = 0b101
  
  TEMP_CTRL = 5
  PRESS_CTRL = 2
  SENS_MODE = 0
  
  HUM_MASK = 0b00000111
  TEMP_MASK = 0b11100000
  PRESS_MASK = 0b00011100
  MODE_MASK = 0b00000011
  SLEEP_MODE = 0b00
  FORCED_MODE = 0b01
  NORMAL_MODE = 0b11

  STDBY_MASK = 0b11100000
  STDBY_TIME = 5
  IIR_MASK = 0b00011100
  IIR_FILT = 2
  
  HALF_MS = 0b000
  SIXTYTWODOTFIVE_MS = 0b001
  HUNDREDTWENTYFIVE_MS = 0b010
  TWOHUNDREDSFIFTY_MS = 0b011
  FIVEHUNDREDS_MS = 0b100
  THOUSAND_MS = 0b101
  TEN_MS = 0b110
  TWENTY_MS = 0b111
  
  IIR_OFF = 0b000
  IIR_2 = 0b001
  IIR_4 = 0b010
  IIR_8 = 0b011
  IIR_16 = 0b100

  @staticmethod
  def int_from_bytes(b, endian = sys.byteorder, signed=False) :
    value = int.from_bytes(b, endian)
    if not signed :
      return value
    width = len(b) * 8
    if value > (1 << (width - 1)) -1 :
      value -= (1 << width)
    return value
  
  def __init__(self, i2c_num, address = 0x76) :
    self.i2c = pyb.I2C(i2c_num, pyb.I2C.MASTER)
    self.addr = address
    if not self.i2c.is_ready(self.addr) :
      raise ValueError('No I2C device on I2C bus ' + str(i2c_num) + ' at address ' + str(self.addr))
      
    self.id = BME.int_from_bytes(self.i2c.mem_read(1, self.addr, self.ID_REG))
    
    buf = bytearray(2)

    self.i2c.mem_read(buf, self.addr, BME.DIG_T1)
    self.dig_T1 = BME.int_from_bytes(buf, "little")
    self.i2c.mem_read(buf, self.addr, BME.DIG_T2)
    self.dig_T2 = BME.int_from_bytes(buf, "little", signed = True)
    self.i2c.mem_read(buf, self.addr, BME.DIG_T3)
    self.dig_T3 = BME.int_from_bytes(buf, "little", signed = True)
    
    self.i2c.mem_read(buf, self.addr, BME.DIG_P1)
    self.dig_P1 = BME.int_from_bytes(buf, "little")
    self.i2c.mem_read(buf, self.addr, BME.DIG_P2)
    self.dig_P2 = BME.int_from_bytes(buf, "little", signed = True)
    self.i2c.mem_read(buf, self.addr, BME.DIG_P3)
    self.dig_P3 = BME.int_from_bytes(buf, "little", signed = True)
    self.i2c.mem_read(buf, self.addr, BME.DIG_P4)
    self.dig_P4 = BME.int_from_bytes(buf, "little", signed = True)
    self.i2c.mem_read(buf, self.addr, BME.DIG_P5)
    self.dig_P5 = BME.int_from_bytes(buf, "little", signed = True)
    self.i2c.mem_read(buf, self.addr, BME.DIG_P6)
    self.dig_P6 = BME.int_from_bytes(buf, "little", signed = True)
    self.i2c.mem_read(buf, self.addr, BME.DIG_P7)
    self.dig_P7 = BME.int_from_bytes(buf, "little", signed = True)
    self.i2c.mem_read(buf, self.addr, BME.DIG_P8)
    self.dig_P8 = BME.int_from_bytes(buf, "little", signed = True)
    self.i2c.mem_read(buf, self.addr, BME.DIG_P9)
    self.dig_P9 = BME.int_from_bytes(buf, "little", signed = True)
    
    self.dig_H1 = BME.int_from_bytes(self.i2c.mem_read(1, self.addr, BME.DIG_H1), "little")
    self.i2c.mem_read(buf, self.addr, BME.DIG_H2)
    self.dig_H2 = BME.int_from_bytes(buf, "little", signed = True)
    self.dig_H3 = BME.int_from_bytes(self.i2c.mem_read(1, self.addr, BME.DIG_H3), "little")
    self.i2c.mem_read(buf, self.addr, BME.DIG_H4)
    self.dig_H4 = (buf[0] << 4) | (buf[1] & 0b1111)
    self.i2c.mem_read(buf, self.addr, BME.DIG_H5)
    self.dig_H5 = (buf[1] << 4) | ((buf[0] >> 4) & 0b1111)
    self.dig_H6 = BME.int_from_bytes(self.i2c.mem_read(1, self.addr, BME.DIG_H6), "little", signed = True)
    
    self.i2c.mem_write(BME.OVRSAMP_1, self.addr, BME.HUM_CTRL_REG)
    buf = bytearray(1)
    self.i2c.mem_read(buf, self.addr, BME.MEAS_REG)
    buf[0] &= ~(BME.TEMP_MASK | BME.PRESS_MASK)
    buf[0] |= (BME.OVRSAMP_1 << BME.TEMP_CTRL) \
            | (BME.OVRSAMP_1 << BME.PRESS_CTRL)
    self.i2c.mem_write(buf, self.addr, BME.MEAS_REG)
 

  def normalmode(self, standby = 0) :
    buf = bytearray(1)
    self.i2c.mem_read(buf, self.addr, BME.MEAS_REG)
    buf[0] &= ~BME.MODE_MASK
    buf[0] |= BME.NORMAL_MODE
    self.i2c.mem_write(buf, self.addr, BME.MEAS_REG)
    if standby < 0 or standby > BME.TWENTY_MS :
        standby = BME.HALF_MS
    self.i2c.mem_read(buf, self.addr, BME.CONF_REG)
    buf[0] &= ~BME.STDBY_MASK
    buf[0] |= (standby << BME.STDBY_TIME)
    self.i2c.mem_write(buf, self.addr, BME.CONF_REG)
    
  def filtering(self, coef) :
    if not coef in [BME.IIR_OFF, BME.IIR_2, BME.IIR_4, BME.IIR_8, BME.IIR_16] :
      raise ValueException("IIR coefficient " + str(coef) + " is invalid")
    buf = bytearray(1)
    self.i2c.mem_read(buf, self.addr, BME.CONF_REG)
    buf[0] &= ~BME.IIR_MASK
    buf[0] |= (coef << BME.IIR_FILT)
    self.i2c.mem_write(buf, self.addr, BME.CONF_REG)

  def humidity_mode(self, mode) :
    if not mode in [BME.SKIP, BME.OVRSAMP_1, BME.OVRSAMP_2, BME.OVRSAMP_4, BME.OVRSAMP_8, BME.OVRSAMP_16] :
      raise ValueException("Mode " + str(mode) + " is invalid")
    buf = bytearray(1)
    self.i2c.mem_read(buf, self.addr, BME.HUM_CTRL_REG)
    buf[0] &= ~BME.HUM_MASK
    buf[0] |= mode
    self.i2c.mem_write(buf, self.addr, BME.HUM_CTRL_REG)
    
  def temperature_mode(self, mode) :
    if not mode in [BME.SKIP, BME.OVRSAMP_1, BME.OVRSAMP_2, BME.OVRSAMP_4, BME.OVRSAMP_8, BME.OVRSAMP_16] :
      raise ValueException("Mode " + str(mode) + " is invalid")
    buf = bytearray(1)
    self.i2c.mem_read(buf, self.addr, BME.MEAS_REG)
    buf[0] &= ~BME.TEMP_MASK
    buf[0] |= (mode << BME.TEMP_CTRL)
    self.i2c.mem_write(buf, self.addr, BME.MEAS_REG)
    
  def pressure_mode(self, mode) :
    if not mode in [BME.SKIP, BME.OVRSAMP_1, BME.OVRSAMP_2, BME.OVRSAMP_4, BME.OVRSAMP_8, BME.OVRSAMP_16] :
      raise ValueException("Mode " + str(mode) + " is invalid")
    buf = bytearray(1)
    self.i2c.mem_read(buf, self.addr, BME.MEAS_REG)
    buf[0] &= ~BME.PRESS_MASK
    buf[0] |= (mode << BME.PRESS_CTRL)
    self.i2c.mem_write(buf, self.addr, BME.MEAS_REG)
  
  def raw_measure(self, buf) :
    self.i2c.mem_read(buf, self.addr, self.PRESS_REG)
    return buf

  def compensation(self, buf, results = None) :
    if results == None :
      results = {}
    
    press = (buf[0] << 16 | buf[1] << 8 | buf[2]) >> 4
    temp = (buf[3] << 16 | buf[4] << 8 | buf[5]) >> 4
    hum = buf[6] << 8 | buf[7]

    v1 = (((temp >> 3) - (self.dig_T1 << 1)) * (self.dig_T2)) >> 11
    v2 = (temp >> 4) - self.dig_T1
    v2 = (((v2 * v2) >> 12) * self.dig_T3) >> 14
    t_fine = v1 + v2
    
    temp = (t_fine * 5 + 128) >> 8
    
    v1 = t_fine - 128000
    v2 = v1 * v1 * self.dig_P6
    v2 += (v1 * self.dig_P5) << 17
    v2 += (self.dig_P4 << 35)
    v1 = ((v1 * v1 * self.dig_P3) >> 8) + ((v1 * self.dig_P2) << 12)
    v1 = (((1 << 47) + v1) * self.dig_P1) >> 33
    if v1 == 0 :
      p = 0
    else :
      p = 1048576 - press
      p = (((p << 31) - v2) * 3125) // v1
      v1 = (self.dig_P9 * (p >> 13) * (p >> 13)) >> 25
      v2 = (self.dig_P8 * p) >> 19
      p = ((p + v1 + v2) >> 8) + (self.dig_P7 << 4)
      p //= 256
    
    h = t_fine - 76800
    h = (((hum << 14) - (self.dig_H4 << 20) - (self.dig_H5 * h) + 16384) >> 15) \
      * (((((((h * self.dig_H6) >> 10) * (((h * self.dig_H3) >> 11) + 32768)) >> 10) + 2097152) * self.dig_H2 + 8192) >> 14)
    h -= (((((h >> 15) * (h >> 15)) >> 7) * self.dig_H1) >> 4)
    if h < 0 :
      h = 0
    if h > 419430400 :
      h = 419430400
    h >>= 12
    h *= 100
    h //= 1024
      
    results['temp'] = temp
    results['press'] = p
    results['hum'] = h
    return results
  
  def measure(self) :
    buf = bytearray(8)
    self.raw_measure(buf)
    return self.compensation(buf)

  def measure_text(self, data_list, index_p, index_h):
    buf = bytearray(8)
    self.raw_measure(buf)
    m = self.compensation(buf)
    pr = m["press"] / 100
    hu = m["hum"] / 100
    pr_str = str("%.2f" % pr)
    pr_str = pr_str.replace('.', ',')
    hu_str = str("%.1f" % hu)
    hu_str = hu_str.replace('.', ',')
    data_list[index_p] = pr_str 
    data_list[index_h] = hu_str
    
#end of file