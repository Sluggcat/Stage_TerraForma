'''
Mesure de la pression, l'humidité et la température
Format :
'''
from BME280 import BME
import utime

def boopy_ptu():
    try:
        bme = BME(4, 0x76)
        sensor_ok = True
    except:
        sensor_ok = False
        
    if sensor_ok == True:
        bme.humidity_mode(BME.OVRSAMP_16)
        bme.temperature_mode(BME.OVRSAMP_16)
        bme.pressure_mode(BME.OVRSAMP_16)
        bme.normalmode(BME.HUNDREDTWENTYFIVE_MS)
        bme.filtering(BME.IIR_16) 
        
        m = bme.measure()
        utime.sleep(1) # Rustine pour avoir une mesure correcte :
        m = bme.measure() # Doubler la mesure 
        p = m['press'] // 10
        #t = m['temp'] 
        u = m['hum'] // 10
        
        return p, u
    else:
        return 0, 0
