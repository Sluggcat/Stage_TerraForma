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