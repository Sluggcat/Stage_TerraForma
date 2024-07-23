"""
data_transmitter.py

This module contains classes for handling UART data reception and LoRa data transmission.

Classes:
- UARTReceiver: Handles receiving float data over UART, converting it to a bytearray.
- LoRaSender: Handles sending bytearray data over LoRa.
- DataTransmitter: Integrates UARTReceiver and LoRaSender to receive data from UART and send it over LoRa.

Configuration for UART and LoRa is provided through the config.py module.
"""

import uasyncio as asyncio
import machine
import struct
from machine import Pin
import config
import ubinascii

class UARTReceiver:
    def __init__(self):
        self.uart = config.initialize_UART()
        self.received_data = []
        self.received = False

    async def receive_uart_data(self):
        while not received:
            if self.uart.any():
                data = self.uart.read()
                if not (data is None):
                    float_strs = data.decode("utf-8").strip().split("\n")
                    floats = [float(f) for f in float_strs]
                    self.received_data.extend(floats)
                    received = True
            await asyncio.sleep(1)

    def get_bytearray_data(self):
        byte_array = bytearray()
        for f in self.received_data:
            byte_array.extend(bytearray(struct.pack('f', f)))
        self.received_data.clear()  # Clear received data after converting
        return byte_array

class LoRaSender:
    def __init__(self):
        self.lora = config.initialize_lora()

    def send(self, data):
        print("Sending packet...", self.lora.frame_counter, ubinascii.hexlify(data), "\n")
        self.lora.send_data(data, len(data), self.lora.frame_counter)
        #self.lora.frame_counter += 1
        print(len(data), "bytes sent!\nframe_counter: ", self.lora.frame_counter)
        
    def loraBitEncoder(self):
        size_data_send = config.initialize_format_data() 
        print(len(size_data_send))

class DataTransmitter:
    def __init__(self):
        self.uart_receiver = UARTReceiver()
        self.lora_sender = LoRaSender()

    async def run(self):
        asyncio.create_task(self.uart_receiver.receive_uart_data())

        while True: # self.lora_sender.lora.frame_counter <= 3:
            if self.uart_receiver.received_data:
                byte_array = self.uart_receiver.get_bytearray_data()
                self.lora_sender.send(byte_array)
                
            await asyncio.sleep(2)
