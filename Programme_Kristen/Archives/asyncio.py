import uasyncio as asyncio
import utime

async def main():
    print('Hello ...')
    await asyncio.sleep(1)
    print('... World!')

async def second():
    while True:
        print("la....")
        await asyncio.sleep_ms(100)

asyncio.run(second())
asyncio.run(main())