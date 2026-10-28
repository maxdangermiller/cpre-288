import time # Time library   
import socket   


HOST = "192.168.1.1" 
PORT = 288 

cybot_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
cybot_socket.connect((HOST, PORT))
cybot_socket.settimeout(0.5)
                      
cybot = cybot_socket.makefile("rbw", buffering=0)

# cybot.write("send_message".encode())

cmd = ' '

while (cmd != 'e'):
    cybot.write("h".encode())

    try:
        rx_message = cybot.readline()
        print("[ CYBOT ] " + rx_message.decode())
    except OSError:
        pass

    # cmd = input("Command to send to CyBot: ")

    if len(cmd) == 0:
        cmd = ' '
    elif len(cmd) > 1:
        cmd = cmd[0]
    
    if cmd == 'm' or cmd == 'h':
        pass


print("Client exiting, and closing file descriptor, and/or network socket\n")
time.sleep(2) # Sleep for 2 seconds
cybot.close()
cybot_socket.close()
