"""
    Communication manager library by Sluggcat
"""
# Modifier ce dictionnaire pour config. ABP de LoRaWAN
ttn_config ={
    # Adresse du dispositif : BAC00001
    'devadd': bytearray([0x42, 0x41, 0x43, 0x40, 0x30, 0x30, 0x30, 0x31]),
    
    # Clé de session du réseau : 9f2b3c4d5e6f708192a1b2c3d4e5f607
    'nwkey': bytearray([
        0x9f, 0x2b, 0x3c, 0x4d, 0x5e, 0x6f, 0x70 , 0x81,
        0x92, 0xa1, 0xb2, 0xc3, 0xd4, 0xe5, 0xf6 , 0x07
    ]),
    
    # Clé de session de l application : f2d4e7c8a1b2c3d4e5f6078a9b0c1d2e
    'app': bytearray([
        0xf2, 0xd4, 0xe7, 0xc8, 0xa1, 0xb2, 0xc3, 0xd4,
        0xe5, 0xf6, 0x07, 0x8a, 0x9b, 0x0c, 0x1d, 0x2e
    ]),
    
    # Région
    'country': 'EU',
}

# Classe TTN pour manipuler les clés dans le programme
class TTN:
    def __init__(self, dev_add, net_key, app_key, country='EU'):
        self.dev_add = dev_add
        self.app_key = app_key
        self.net_key = net_key
        self.region = country
        
    @property
    def device_address(self):
        return self.dev_add
    
    @property
    def network_key(self):
        return self.net_key
    
    @property
    def application_key(self):
        return self.app_key
    
    @property
    def country(self):
        return self.region
    
    