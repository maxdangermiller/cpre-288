import time # Time library   
import socket   


HOST = "127.0.0.1" 
PORT = 65432 

cybot_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
cybot_socket.connect((HOST, PORT))
                      
cybot = cybot_socket.makefile("rbw", buffering=0)

# cybot.write("send_message".encode())




print("Client exiting, and closing file descriptor, and/or network socket\n")
time.sleep(2) # Sleep for 2 seconds
cybot.close()
cybot_socket.close()
