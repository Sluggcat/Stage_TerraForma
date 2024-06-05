from MPU6050 import MPU
import utime


def boopy_mpu():
    try:
        mpu = MPU(4, 0x68)
        sensor_ok = True
    except:
        sensor_ok = False
    
    if sensor_ok == True:
        max_ax = 0
        max_ay = 0
        max_az = 0
        max_gx = 0
        max_gy = 0
        max_gz = 0
        for count in range(10):
            (ax, ay, az, tp, gx, gy, gz) = mpu.get_data()
            if ax > max_ax:
                max_ax = ax
            if ay > max_ay:
                max_ay = ay
            if az > max_az:
                max_az = az
                
            if gx > max_gx:
                max_gx = gx
            if gy > max_gy:
                max_gy = gy
            if gz > max_gz:
                max_gz = gz
                
            utime.sleep_ms(200)
        
        ax = int(max_ax)
        ay = int(max_ay)
        az = int(max_az)

        gx = int(max_gx)
        gy = int(max_gy)
        gz = int(max_gz)
        
        return ax, ay, az, gx, gy, gz
    else:
        return 0, 0, 0, 0, 0, 0
