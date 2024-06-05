import uasyncio as asyncio
import ustruct
from pyb import UART
from primitives import Delay_ms



class IRID():
    def __init__(self):
        self.uart = UART(2, 19200)
        self.swriter = asyncio.StreamWriter(self.uart, {})
        self.sreader = asyncio.StreamReader(self.uart)
        self.response = []
        self.delay = Delay_ms()
        self.timeout = 20000
        self.count = 4
        asyncio.create_task(self._recv())

    def uart_off(self):
        self.uart.deinit()
        
    async def _recv(self):
        while True:
            res = await self.sreader.readline()
            self.response.append(res)
            self.delay.trigger(self.timeout)    
             
    async def send(self, cmd, t):
        self.response = []
        self.timeout = t * 1000
        await self.swriter.awrite("{}\r".format(cmd))
        self.delay.trigger(self.timeout)
        while self.delay.running():
            await asyncio.sleep(1)
        return self.response

    async def send_bin(self, dframe, t):
        self.response = []
        self.timeout = t * 1000
        cs = 0
        for i in range(len(dframe)):
            cs += int(dframe[i])
            await self.swriter.awrite(ustruct.pack(">b", int(dframe[i])))   
        
        await self.swriter.awrite(ustruct.pack(">h", cs))
        
        self.delay.trigger(self.timeout)
        while self.delay.running():
            await asyncio.sleep(1)
        return self.response

# end of file