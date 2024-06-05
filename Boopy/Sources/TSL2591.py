# TSL lux sensor interface
import pyb
import utime


class TSL:
    VISIBLE = 2
    INFRARED = 1
    FULLSPECTRUM = 0

    # REGISTERS
    REG_CMD = 0xA0
    REG_ENABLE = 0x00
    REG_CONFIG = 0x01
    REG_PID = 0x11
    REG_ID = 0x12
    REG_STATUS = 0x13
    REG_C0DATAL = 0x14
    REG_C0DATAH = 0x15
    REG_C1DATAL = 0x16
    REG_C1DATAH = 0x17

    # ENABLE register bits values
    EN_POWERON = 0x01
    EN_POWEROFF = 0x00
    EN_AEN = 0x02
    EN_AIEN = 0x10
    EN_NPIEN = 0x80

    INTEGRATIONTIME_100MS = 0x00
    INTEGRATIONTIME_200MS = 0x01
    INTEGRATIONTIME_300MS = 0x02
    INTEGRATIONTIME_400MS = 0x03
    INTEGRATIONTIME_500MS = 0x04
    INTEGRATIONTIME_600MS = 0x05

    GAIN_LOW = 0x00
    GAIN_MED = 0x10
    GAIN_HIGH = 0x20
    GAIN_MAX = 0x30

    @staticmethod
    def bytes_to_int(data):
        return data[0] + (data[1] << 8)

    def __init__(
        self, i2c_num, address=0x29, integration=INTEGRATIONTIME_100MS, gain=GAIN_LOW
    ):
        self.i2c = pyb.I2C(i2c_num, pyb.I2C.MASTER)
        self.addr = address
        if not self.i2c.is_ready(self.addr):
            raise ValueError(
                "No I2C device on I2C bus "
                + str(i2c_num)
                + " at address "
                + str(self.addr)
            )
        self.integration_time = integration
        self.gain = gain
        self.set_timing(self.integration_time)
        self.set_gain(self.gain)
        self.disable()

    def enable(self):
        buf = bytearray(1)
        buf[0] = TSL.EN_POWERON | TSL.EN_AEN | TSL.EN_AIEN
        self.i2c.mem_write(buf, self.addr, TSL.REG_CMD | TSL.REG_ENABLE)

    def disable(self):
        buf = bytearray(1)
        buf[0] = TSL.EN_POWEROFF
        self.i2c.mem_write(buf, self.addr, TSL.REG_CMD | TSL.REG_ENABLE)

    def set_timing(self, integration):
        buf = bytearray(1)
        self.enable()
        self.integration_time = integration
        self.atime = (integration + 1) * 100
        buf[0] = integration | self.gain
        self.i2c.mem_write(buf, self.addr, TSL.REG_CMD | TSL.REG_CONFIG)
        self.disable()

    def set_gain(self, gain):
        buf = bytearray(1)
        self.enable()
        self.gain = gain
        self.again = 1.0
        if gain == TSL.GAIN_MED:
            self.again = 25.0
        elif gain == TSL.GAIN_HIGH:
            self.again = 428.0
        elif gain == TSL.GAIN_MAX:
            self.again = 9876.0
        buf[0] = gain | self.integration_time
        self.i2c.mem_write(buf, self.addr, TSL.REG_CMD | TSL.REG_CONFIG)
        self.disable()

    def get_full_luminosity(self):
        buf = bytearray(2)
        self.enable()
        utime.sleep_ms(self.atime + 20)
        self.i2c.mem_read(buf, self.addr, TSL.REG_CMD | TSL.REG_C0DATAL)
        full = TSL.bytes_to_int(buf)
        self.i2c.mem_read(buf, self.addr, TSL.REG_CMD | TSL.REG_C1DATAL)
        ir = TSL.bytes_to_int(buf)
        self.disable()
        return full, ir

    def get_lux(self):
        if self.integration_time == TSL.INTEGRATIONTIME_100MS:
            max_counts = 36863
        else:
            max_counts = 65536
        full, ir = self.get_full_luminosity()
        if full < max_counts and ir < max_counts:
            cpl = (self.atime * self.again) / 408.0
            lux1 = (full - (1.64 * ir)) / cpl
            lux2 = ((full * 0.59) - (ir * 0.86)) / cpl
        else:
            lux1 = -1
            lux2 = -1
        res = max(lux1, lux2)
        res = round(res * 1.15)
        return res
