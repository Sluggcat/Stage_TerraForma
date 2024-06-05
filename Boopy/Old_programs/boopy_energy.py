from INA219 import INA
import utime

def boopy_solar():
    try:
      solar = INA(4, 0x44)
    except:
      return -1, 0
    cur = []
    for i in range(10):
        cur.append(solar.current)
        utime.sleep(0.1)
    current = max(cur)
    return solar.voltage, current

def boopy_battery():
    try:
        batt = INA(4, 0x41)
    except:
        return -1, 0
    cur = []
    for i in range(10):
        cur.append(batt.current)
        utime.sleep(0.1)
    current = max(cur)
    return batt.voltage, current

def boopy_user():
    try:
        usr = INA(4, 0x40)
    except:
        return -1, 0
    cur = []
    for i in range(10):
        #volt, current = boopy_user()
        cur.append(usr.current)
        utime.sleep(0.1)
    current = max(cur)
    return usr.voltage, current

def energy_init():
    volt, current = boopy_solar()
    volt, current = boopy_battery()
    volt, current = boopy_user()
    
def ina_param():
    usr = INA(4, 0x40)
    #print("{0} {1} {2} {3}\n".format(usr.conf, usr.shunt_v, usr.bus_v, usr.calib))
    #print("{0} {1}".format(usr.shunt_v, usr.bus_v))
    #print("{0}\n".format(usr.calib))
    print("{0} {1}".format(usr.voltage / 1000, usr.shunt_v / 1000 ))
    '''
    print(usr.conf)
    print(usr.shunt_v)
    print(usr.bus_v)
    print(usr.calib)
    '''    
    