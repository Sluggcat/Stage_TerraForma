"""
    Asynchronous UART Data Reception Script
    This script receives float data over UART using an asynchronous background function.
    Data is read from the UART port, converted to float values, and stored in a global list.
    The main loop periodically processes and prints the received float data.
    
    by Sluggcat
"""

import uasyncio as asyncio
import machine
import struct

# UART configuration (adjust pins and baudrate as needed)
uart = machine.UART(2, baudrate=9600, tx=17, rx=16)  # UART2, tx=17, rx=16 for ESP32

# Global variable to store received data
received_data = []

# Function to handle UART data reception
async def receive_uart_data():
    while True:
        if uart.any():
            data = uart.read()
            if data:
                float_strs = data.decode().strip().split("\n")
                # Convert the string representations of floats to actual floats
                floats = [float(f) for f in float_strs]
                # Extend received_data with new floats
                received_data.extend(floats)
        await asyncio.sleep(1)
        
# Main function to process received data
async def main():
    print("===\nMain program starting")
    # Start UART data reception coroutine
    asyncio.create_task(receive_uart_data())

    while True:
        if received_data:
            print("\nReceived data as floats:\n\t", received_data)
            # Clear received_data after processing
            received_data.clear()
        else:
            print("\nNo data received")
        await asyncio.sleep(2)  # Adjust sleep time as needed
        
# Run asyncio event loop
loop = asyncio.get_event_loop()
try:
    loop.run_until_complete(main())
except KeyboardInterrupt:
    print("Interrupted")
finally:
    # Cleanup code can go here, if needed
    loop.close()
