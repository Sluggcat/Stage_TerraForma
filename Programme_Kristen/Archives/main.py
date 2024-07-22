import time

#---- Taille des données traitées
size_value = [7, 8, 13, 6, 8, 5, 8, 5, 8, 11, 8, 11, 11, 11, 11, 11]
command_size = [1, 7, 32, 16, 24, 16, 24, 8, 8, 4]

#---- Function to decode LoRa message
def byte_to_int(data):
    binary_list = []
    raw_command = ''
    command = [''] * len(command_size)
    for i in range(len(data)):
        binary_list.append(bin(int(data[i])))
        binary_list[i] = binary_list[i][2:]
        for j in range(8 - len(binary_list[i])):
            binary_list[i] = "0" + binary_list[i]
        raw_command = raw_command + binary_list[i]
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
    bit_run = 0
    missing_bit = 0
    hexa_number = 0
    bloc_bin = [0] * l
    hexa = []
    byte = []
#---- Changing the format : int to binary (string)
    for i in range(l):
        #---- bloc_bin is the binary values of bloc
        bloc_bin[i] = bin(bloc[i])
        #---- Delete the prefix 0b at the begining of each string
        bloc_bin[i] = bloc_bin[i][2:]
        for j in range(size_value[i] - len(bloc_bin[i])):
            bloc_bin[i] = "0" + bloc_bin[i]
        #---- Print to debug
        #print(str(bloc_bin[i]) + " - " + str(len(bloc_bin[i])))
    #print(str(bloc_bin))
#---- Converting binary to byte
    for i in range(l):
        #print("-------------------------------------------------")
        if (missing_bit != 0):
            #print("----> Boucle complément " + str(i))
            for j in range(missing_bit):
                hexa_number += int(bloc_bin[i][len(bloc_bin[i]) - (j+1)]) * 2**(j + (4 - missing_bit))
                #print("boucle n°" + str(j))
            bit_run = missing_bit
            missing_bit = 0
            hexa.append(hex(hexa_number))
            hexa_number = 0
            #print("bit_run : " + str(bit_run))
        while ((len(bloc_bin[i]) - bit_run) >= 4) & (missing_bit == 0):
            #print("----> Boucle While " + str(i))
            for j in range(4):
                hexa_number += int(bloc_bin[i][len(bloc_bin[i]) - (1+j+bit_run)]) * 2**j
                #print("boucle n°" + str(j))
            bit_run += 4
            hexa.append(hex(hexa_number))
            hexa_number = 0
            #print("bit_run : " + str(bit_run))
        if bit_run == len(bloc_bin[i]):
            #print("----> Boucle complete " + str(i))
            bit_run = 0
            missing_bit = 0
            #print("bit_run : " + str(bit_run))
        else:
            #print("----> Boucle bit manquant " + str(i))
            missing_bit = len(bloc_bin[i]) - bit_run
            for j in range(missing_bit):
                hexa_number += int(bloc_bin[i][len(bloc_bin[i]) - (1+j+bit_run)]) * 2**j
                #print("boucle n°" + str(j))
            missing_bit = 4 - missing_bit
            if i == l - 1:
                hexa.append(hex(hexa_number))
            #print("missing : " + str(missing_bit))
            bit_run = 0
            #print("bit_run : " + str(bit_run))
    for i in range(len(hexa)//2):
        byte.append(hexa[i*2] + hexa[(i*2)+1][2:])
    if len(hexa)%2 != 0:
        byte.append(hexa[len(hexa)-1] + "0")
    return byte
    
a = time.ticks_ms()
bob = [16, 53, 3924, 19, 66 ,0 , 25, 3, 99, 337, 17, 236, 458, 188, 52, 108]
bobis = int_to_byte(bob)
#print(len(bob))

print(int_to_byte(bob))
b = time.ticks_ms()
print("time = " + str(b-a))

a = time.ticks_ms()
print(byte_to_int(bobis))
b = time.ticks_ms()
print("time = " + str(b-a))


