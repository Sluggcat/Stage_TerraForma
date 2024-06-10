'''
BOOPY V2 - version 4.6 du 231005:
Test en mer à Banyuls
'''
import utime
import ustruct
import uasyncio as asyncio
import uos # debug sur micro sd
from pyb import SDCard
from pyb import Pin
from GPS import GPS
from RTC import RTC
from pyb import UART
from pyb import LED
from boopy_temp import boopy_temp10  # Capteur de température interne
from boopy_ptu import boopy_ptu   # Capteur de pression et humidité
from boopy_mpu import boopy_mpu  # Centrale inertielle
from boopy_extern import extern_temp10  # Capteurs externes
from boopy_energy import boopy_solar, boopy_battery, boopy_user
from iridium import IRID
from main_sd import main_sd
from main_irid import main_irid


async def main():
    main_pwr = Pin("MAIN_PWR", Pin.OUT_PP) 
    main_pwr.on()
    utime.sleep(1)
    asyncio.run(main_sd())
asyncio.run(main())

# End of file