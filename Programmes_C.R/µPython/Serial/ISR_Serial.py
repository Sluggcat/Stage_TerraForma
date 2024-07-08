import machine
import time
import struct

# UART configuration (adjust pins and baudrate as needed)
uart = machine.UART(2, baudrate=9600, tx=17, rx=16)  # UART2, tx=17, rx=16 for ESP32

# Global variables to store received data
received_data = []

# ISR function to handle UART data reception
def uart_isr(arg):
    global received_data
    while uart.any():
        data = uart.read()
        if data:
            # Decode and split data by line breaks (assuming floats are sent as text with line breaks)
            float_strs = data.decode().strip().split("\n")
            # Convert the string representations of floats to actual floats
            floats = [float(f) for f in float_strs]
            # Extend received_data with new floats
            received_data.extend(floats)

# Attach UART interrupt
uart.irq(handler=uart_isr, trigger=machine.UART.RX_ANY)

while True:
    if received_data:
        print("Received data as floats:", received_data)
        # Clear received_data after processing
        received_data = []
    else:
        print("No data received")
    
    # Add a small delay to avoid spamming the UART (sync with data arrival rate)
    time.sleep(2)
