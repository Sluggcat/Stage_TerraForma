"""
    UART Data Matching Test Script
    This script checks if the data received by the ESP32 matches the frame sent by the probe.
    It reads float data from the UART port, converts it to a byte array, and verifies the received data.
    
    by Sluggcat
"""

import machine
import time
import struct

# UART configuration (adjust pins and baudrate as needed)
uart = machine.UART(2, baudrate=9600, tx=17, rx=16)  # UART2, tx=17, rx=16 for ESP32

def receive_float_data():
    # Wait for data to become available
    while not uart.any():
        pass
    
    # Read all available data from UART
    data = uart.read()
    
    if data:
        # Split the data by linebreak (assuming floats are sent as text with line breaks)
        float_strs = data.decode().strip().split("\n")
        
        # Convert the string representations of floats to actual floats
        floats = [float(f) for f in float_strs]
        
        # Convert floats back to bytes and store in a bytearray
        byte_array = bytearray()
        for f in floats:
            byte_array.extend(bytearray(struct.pack('f', f)))
        
        return floats  # Test data received
        # return byte_array  # Frame to send by LoRA
    
    else:
        return None

while True:
    # Receive float data from UART and convert to bytearray
    received_data = receive_float_data()
    
    if received_data:
        print("Received data as bytearray:", received_data, "\n")
    else:
        print("No data received")
    
    # Add a small delay to avoid spamming the UART (make sure to sync with the probe)
    time.sleep(2)
