# Copyright 2021 LeMaRiva|tech lemariva.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# PseudoBoopy ESP32
device_config = {
    'spi_unit': 1,
    'miso':19,
    'mosi':18,
    'ss':33,
    'sck':5,
    'reset':32,
    'led':2, 
}

app_config = {
    'loop': 200,
    'sleep': 100,
}

lora_parameters = {
    'tx_power_level': 2, 
    'signal_bandwidth': 'SF7BW125',
    'spreading_factor': 7,    
    'coding_rate': 5, 
    'sync_word': 0x34, 
    'implicit_header': False,
    'preamble_length': 8,
    'enable_CRC': True,
    'invert_IQ': False,
}

wifi_config = {
    'ssid':'',
    'password':''
}

ttn_config = {
    'devaddr': bytearray([0xBA, 0xC2, 0x02, 0x00]),
    'nwkey': bytearray([
        0x9F, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x70, 0x81,
        0x92, 0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07
        ]),
    'app': bytearray([
        0xF2, 0xD4, 0xE7, 0xC8, 0xA1, 0xB2, 0xC3, 0xD4,
        0xE5, 0xF6, 0x07, 0x8A, 0x9B, 0x0C, 0x1D, 0x2E
        ]),
    'country': 'EU',
}