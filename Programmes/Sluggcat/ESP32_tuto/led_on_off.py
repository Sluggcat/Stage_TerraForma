"""
    Standalone test : basic GPIO output control
        Sluggcat
"""

from machine import Pin # importation de la classe utile uniquement

led = Pin(2, Pin.OUT) # déclaration du port lié à la LED

led.value(0) 
# 0: LED off   # 1: LED on
