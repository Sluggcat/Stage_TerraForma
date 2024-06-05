from MCP9808 import MCP  # Capteur de température MCP9808
import utime

def boopy_temp():
    try:
        mcp = MCP(4, 0x18)
        sensor_ok = True
    except:
        sensor_ok = False
        
    if sensor_ok == True:
        mcp.set_res(mcp.RES_MAX)
        
        # Moyenne de 10 mesures 
        tent=0
        tdec=0
        for t in range(10):
            (e,d) = mcp.get_temp()
            tent+=e
            tdec+=d
            utime.sleep_ms(100)
        ent = tent // 10  # Partie entière de la temperature
        dec = tdec // 10  # Partie décimale au centième de deg

        # Arrondi au dixième de degré
        ddec = (dec + 5) // 10
        
        return ent * 10 + ddec
    else:
        return 0
    
def boopy_temp10():
    try:
        mcp = MCP(4, 0x18) # 0x18 initale sinon 0x19 fil rouge
        sensor_ok = True
    except:
        sensor_ok = False
    
    if sensor_ok == True:
        mcp.set_res(mcp.RES_MAX)

        # Moyenne de 10 mesures 
        tent=0
        tdec=0
        for t in range(10):
            (e,d) = mcp.get_temp()
            tent+=e
            tdec+=d
            utime.sleep_ms(100)
        ent = tent // 10  # Partie entière de la temperature
        dec = tdec // 10  # Partie décimale au centième de deg

        ddec = (dec + 5) // 10  # Arrondi au dixième de degré
        
        return ent * 10 + ddec    
    else:
        return 0