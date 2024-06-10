'''
Remise à zero des compteurs de fichier micro SD
'''
import utime
import uos # Micro sd
from pyb import SDCard

# ============ F I L E S ================
try:
    uos.mount(SDCard(), "/sd")
except:
    print("Error SD mount")

#--- Initialiser le numero du fichier
ifile = 1

try:
    file = open("/flash/sd_file_cnt.txt", 'w')
    file.write(str(ifile) + '\n')
    file.close()
except:
    print("erreur ecriture fichier file_cnt")
            
#--- Initialiser le numero de la trame
iframe = 0
try:
    file = open("/flash/sd_frame_cnt.txt", "w")
    file.write(str(iframe) + '\n')
    file.close()
except:
    print("Erreur ecriture iframe")            
            
