import uasyncio as asyncio
import ustruct
from pyb import UART



class IRID():
    def __init__(self):
        self.uart = UART(2, 19200)
        self.swriter = asyncio.StreamWriter(self.uart, {})
        self.sreader = asyncio.StreamReader(self.uart)
        self.response = []
        self.count = 4
        asyncio.create_task(self._recv())

    def uart_off(self):
        self.uart.deinit()
        
    async def _recv(self):
        while True:
            res = await self.sreader.readline()
            self.response.append(res)
                
             
    async def send(self, cmd):
        self.response = []
        await self.swriter.awrite("{}\r".format(cmd))
        while self.count>0:
           await asyncio.sleep(1)
           self.count -=1
        return self.response

    async def send_bin(self, dframe):
        self.response = []
        cs = 0
        for i in range(len(dframe)):
            cs += int(dframe[i])
            await self.swriter.awrite(ustruct.pack(">b", int(dframe[i])))   
        
        await self.swriter.awrite(ustruct.pack(">h", cs))
        
        while self.count>0:
           await asyncio.sleep(1)
           self.count -=1
        return self.response
