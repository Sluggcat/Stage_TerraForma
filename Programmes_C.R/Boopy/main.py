"""
main.py

This script serves as the main entry point for the application. 
It initializes the DataTransmitter class, which handles receiving 
data over UART, converting it to a bytearray, and sending it via 
LoRa. The script uses asyncio to run the main event loop. It also 
controls the LED to indicate the running state.
"""

import uasyncio as asyncio
from data_transmitter import DataTransmitter
from machine import Pin
import config

async def run_main():
    data_transmitter = DataTransmitter()
    await data_transmitter.run()

if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    try:
        loop.run_until_complete(run_main())
    except KeyboardInterrupt:
        print("Interrupted")
    finally:
        loop.close()
