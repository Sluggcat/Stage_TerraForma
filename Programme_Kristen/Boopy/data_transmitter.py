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
import utime
import machine
import struct
from machine import Pin
import config
import ubinascii

#floats = [25.64, 1024.0, 12.35, 7.65, 3.14, 250.95, 123.0, 258.0, 365.0, 489.0, 698.0, 720.0, 810.0, 963.0, 1056.0, 1204.0, 1566.0, 1900.0]

class UARTReceiver:
    def __init__(self):
        self.uart = config.initialize_UART()
        self.received_data = []
        self.saving_data = []
        self.size_value = config.initialize_format_data()

    def receive_uart_data(self):
        received = False
        while not received:
            data = self.uart.read()
            if not (data is None):
                print("Received")
                float_strs = data.decode("utf-8").strip().split("\n")
                floats = [float(f) for f in float_strs]
                self.received_data.extend(floats)
                received = True
        utime.sleep(1)
        self.saving_data = floats
        self.received_data = self.float_to_int(self.received_data)

    def float_to_int(self, floats):
        integers = []
        j = 0
        for i in range(len(self.size_value)):
            if self.size_value[i] != 8:
                integers.append(int(floats[i-j]))
            elif self.size_value[i] == 8:
                j += 1
                integers.append(int((floats[i-j]-integers[i-1])*100))
        return integers

    def int_to_byte(self, bloc):
        l = len(bloc)
        hexa_number = 0
        raw_byte = ''
        sign = ''
        bloc_bin = [0] * l
        hexa = []
        byte = []
        #---- Changing to binary
        for i in range(l):
            if bloc[i] >= 0.0:
                bloc_bin[i] = bin(bloc[i])
                bloc_bin[i] = bloc_bin[i][2:]
                sign = sign + "0"
            else:
                bloc_bin[i] = bin(bloc[i])
                bloc_bin[i] = bloc_bin[i][3:]
                sign = sign + "1"
            for j in range(self.size_value[i] - len(bloc_bin[i])):
                bloc_bin[i] = "0" + bloc_bin[i]
            raw_byte = raw_byte + bloc_bin[i]
        raw_byte = raw_byte + sign
        print(raw_byte)
        #---- Creating hexa bloc
        for i in range(len(raw_byte)//4):
            for j in range(4):
                hexa_number += int(raw_byte[j + (i * 4)]) * 2**j
            hexa.append(hex(hexa_number))
            hexa_number = 0
        #---- Completing if zero needed for a complete byte
        for i in range(4):
            if ((len(raw_byte)//4)*4)+i >= len(raw_byte):
                hexa_number += 0
            else:
                hexa_number += int(raw_byte[((len(raw_byte)//4)*4)+i]) * 2**i
        hexa.append(hex(hexa_number))
        hexa_number = 0 
        #---- Creating bytearray  
        for i in range(len(hexa)//2):
            byte.append(int(hexa[i*2] + hexa[(i*2)+1][2:]))
        if len(hexa)%2 != 0:
            byte.append(int(hexa[len(hexa)-1] + "0"))
        print(byte)
        return byte

class LoRaSender:
    def __init__(self):
        self.lora = config.initialize_lora()
        return None

    def send(self, data):
        print("Sending packet...", self.lora.frame_counter, data, "\n")
        #self.lora.send_data(bytearray(data), len(bytearray(data)), self.lora.frame_counter)
        #self.lora.frame_counter += 1
        print(len(data), "bytes sent!\nframe_counter: ", self.lora.frame_counter)

class DataTransmitter:
    def __init__(self):
        self.uart_receiver = UARTReceiver()
        self.lora_sender = LoRaSender()

    def run(self):
        self.uart_receiver.receive_uart_data()

        if self.uart_receiver.received_data != None:
            byte_array = self.uart_receiver.int_to_byte(self.uart_receiver.received_data)
            self.lora_sender.send(byte_array)
            utime.sleep(2)
        
    def getValues(self):
        return str(self.uart_receiver.saving_data)
