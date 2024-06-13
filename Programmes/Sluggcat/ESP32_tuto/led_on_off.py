"""
    Standalone test : contrôle des GPIOS
        Sluggcat
"""

from machine import Pin # librairie pour contrôler les GPIO

led = Pin(2, Pin.OUT) # déclaration du port lié à la LED

led.value(0) # LED off
# led.value(1) # LED on