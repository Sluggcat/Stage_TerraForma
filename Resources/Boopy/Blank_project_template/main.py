"""
BOOPY - version 3 210524
    template pour nouveau projet - forked from v2.1
    by Sluggcat
"""
import utime
import ustruct
import uasyncio as asyncio
from pyb import SDCard
from pyb import Pin
from GPS import GPS
from RTC import RTC
from pyb import UART
from pyb import LED

async def main():