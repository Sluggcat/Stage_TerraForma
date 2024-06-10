from INA219 import INA

def boopy_solar():
    try:
      solar = INA(4, 0x44)
      return solar.voltage, solar.current
    except:
      return -1, 0

def boopy_battery():
    try:
        batt = INA(4, 0x41)
        return batt.voltage, batt.current
    except:
        return -1, 0

def boopy_user():
    try:
        usr = INA(4, 0x40)
        return usr.voltage, usr.current
    except:
        return -1, 0
    
    