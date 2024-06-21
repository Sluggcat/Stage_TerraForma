'''
Mesure de lumière de la bouée Boopy
    Un filtre qui attenu de 25% la lumière incidente
    permet de ne pas saturer le capteur en plein Soleil.
'''
from TSL2591 import TSL  # Capteur de lumière TSL2591

def boopy_light():
    try:
        tsl = TSL(4, 0x29)
        light = tsl.get_lux() * 4  # Puisque filtré à 25%
        return light
    except:
        return 0