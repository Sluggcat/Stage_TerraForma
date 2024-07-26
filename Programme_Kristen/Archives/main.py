import time

#---- Taille des données traitées
size_value = [7, 8, 13, 6, 8, 5, 8, 5, 8, 11, 8, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11]
command_size = [1, 7, 32, 16, 24, 16, 24, 8, 8, 4]
floats = [25.64, 1024.0, 12.35, 7.65, 3.14, 250.95, 123.0, 258.0, 365.0, 489, 698.0, 720.0, 810.0, 963.0, 1056.0, 1204.0, 1566.0, 1900.0]

#---- Function to change float on integer
def float_to_int(floats):
    integers = []
    j = 0
    for i in range(len(size_value)):
        if size_value[i] != 8:
            integers.append(int(floats[i-j]))
        elif size_value[i] == 8:
            j += 1
            integers.append(int((floats[i-j]-integers[i-1])*100))
    return integers

#---- Function to decode LoRa message
def byte_to_int(data):
#---- Create variables
    binary_list = []
    raw_command = ''
    command = [''] * len(command_size)
#---- Creating binary array without prefix
    for i in range(len(data)):
        binary_list.append(bin(int(data[i])))
        binary_list[i] = binary_list[i][2:]
        for j in range(8 - len(binary_list[i])):
            binary_list[i] = "0" + binary_list[i]
        raw_command = raw_command + binary_list[i]
#---- Set array command to send on sensors
    size = 0
    for i in range(len(command_size)):
        for j in range(command_size[i]):
            command[i] = command[i] + (raw_command[size])
            size += 1
        #print("command number " + str(i) + " - " + str(command))
    #print(str(binary_list))
    #print(raw_command)
    return command
    
#---- // Function to convert an integer array into à bytearray
def int_to_byte(bloc):
#---- Initialize variables
    l = len(bloc)
    hexa_number = 0
    raw_byte = ''
    bloc_bin = [0] * l
    hexa = []
    byte = []
#---- Changing the format : int to binary (string)
    for i in range(l):
        #---- bloc_bin is the binary values of bloc
        bloc_bin[i] = bin(bloc[i])
        #---- Delete the prefix 0b at the begining of each string
        bloc_bin[i] = bloc_bin[i][2:]
        #---- Adding zeros to complete binary numbers
        for j in range(size_value[i] - len(bloc_bin[i])):
            bloc_bin[i] = "0" + bloc_bin[i]
        raw_byte = raw_byte + bloc_bin[i]
        #---- Print to debug
        #print(str(bloc_bin[i]) + " - " + str(len(bloc_bin[i])))
    #print("raw_byte : " + str(raw_byte))
#---- Converting binary to byte
    for i in range(len(raw_byte)//4):
        for j in range(4):
            hexa_number += int(raw_byte[j + (i * 4)]) * 2**j
        hexa.append(hex(hexa_number))
        hexa_number = 0
#---- Completing conversion
    for i in range(4):
        if ((len(raw_byte)//4)*4)+i >= len(raw_byte):
            hexa_number += 0
        else:
            hexa_number += int(raw_byte[((len(raw_byte)//4)*4)+i]) * 2**i
    hexa.append(hex(hexa_number))
    hexa_number = 0
#---- Creating bytes    
    for i in range(len(hexa)//2):
        byte.append(hexa[i*2] + hexa[(i*2)+1][2:])
    if len(hexa)%2 != 0:
        byte.append(hexa[len(hexa)-1] + "0")
    return byte
    

print(float_to_int(floats))

a = time.ticks_ms()
#bob = [16, 53, 3924, 19, 66, 0, 25, 3, 99, 337, 17, 236, 458, 188, 52, 108, 123, 159, 204, 365, 545, 681, 789]
bob = float_to_int(floats)
bobis = int_to_byte(bob)
b = time.ticks_ms()
#print(len(bob))

print(int_to_byte(bob))
print("time = " + str(b-a) + "ms")

a = time.ticks_ms()
byte_to_int(bobis)
b = time.ticks_ms()
print(byte_to_int(bobis))
print("time = " + str(b-a) + "ms")



