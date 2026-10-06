# Author: Max Miller
# Date: 10/06/2026

import time
import socket   

# Choose to create either a UART or TCP port socket to communicate with Cybot (Not both!!: I.e, comment out the one not being used)
# UART BEGIN
#cybot = serial.Serial('COM100', 115200)  # UART (Make sure you are using the correct COM port and Baud rate!!)
# UART END

# TCP Socket BEGIN
HOST = "192.168.1.1"
PORT = 288
cybot_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
cybot_socket.connect((HOST, PORT))
                      
cybot = cybot_socket.makefile("rbw", buffering=0)
# TCP Socket END

send_message = "Hello\n"

cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

print("Sent to server: " + send_message) 

# Send messges to server until user sends "quit"
while send_message != 'quit\n':
    print("wait for server reply\n")
    # rx_message = cybot.readline()      # Wait for a message, readline expects message to end with "\n"
    # print("Got a message from server: " + rx_message.decode() + "\n") # Convert message from bytes to String (i.e., decode)
    send_message = input("Enter a message (enter quit to exit):") + '\n' # Enter next message to send to server
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        
print("Client exiting, and closing file descriptor, and/or network socket\n")
time.sleep(2) # Sleep for 2 seconds
cybot.close()         # Close file object associated with the socket or UART
cybot_socket.close()  # Close the socket (NOTE: comment out if using UART interface, only use for network socket option)
