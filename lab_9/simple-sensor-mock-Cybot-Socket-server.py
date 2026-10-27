# Author: Phillip Jones (phjones@iastate.edu)
# Date: 08/08/2023
# Description: Simple Socket mock CyBot sever.  Supporting Sensor Scan server, or "Echo" server, or "Chat" server example

# General Python tutorials (W3schools):  https://www.w3schools.com/python/

# Socket library:  https://realpython.com/python-sockets/  
# See: Background, Socket API Overview, and TCP Sockets  
import socket

# Time library (e.g. sleep(), https://realpython.com/python-time-module/)
import time

import os  # import function for finding absolute path to this python script

# A little python magic to make it more convient for you to ajust where you want the data file to live
# Link for more info: https://towardsthecloud.com/get-relative-path-python 
absolute_path = os.path.dirname(__file__) # Absoult path to this python script
relative_path = "./"   # Path to sensor data file relative to this python script (./ means data file is in the same directory as this python script)
full_path = os.path.join(absolute_path, relative_path) # Full path to sensor data file
filename = 'mock-cybot-sensor-scan.txt' # Name of file containing mock cybot sensor scan data

# TCP Socket BEGIN
HOST = "127.0.0.1"  # Standard loopback interface IP address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)

print("Creating Socket object ")
cybot_server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a socket object

print("Binding socket object to Host: " + HOST + " and Port: " + str(PORT) )
cybot_server_socket.bind((HOST, PORT))  # Bind server to a host and port

print("Listening for a client to connect")
cybot_server_socket.listen()            # Listen for a client to connect to this server

conn, addr = cybot_server_socket.accept()  # Accept a client request to connect to this server, and create a connection
print("Socket Connection from: " + str(addr))  # Display address of client that just connected
# TCP Socket END

while True:  # Loop recieving/sending data from/to client until client disconnects, or error causes data to be 0 (i.e., no data)
        
        try: # Receive data from client.
                data = conn.recv(1024)  # Recieve byte array, will not accept a data packet greater than 1024 bytes             

        except socket.error as msg:  # Client has disconnected, or there has been some type of error while recv waits for data
                print("Connection closed: " + str(msg) + "\n")
                break

        # Check if server has recieved that client is quitting
        if data.decode() == "quit\n": # Client is quitting
                print("Server quitting")
                break

        # Check if a sensor scan command has been sent
        elif (data.decode() == "M\n") or (data.decode() == "m\n"):

                print("Recieved Sensor scan request... sending data:\n")

                # Open file containing mock sensor data
                file_object = open(full_path + filename,'r') # Open the file: file_object is just a variable for the file "handler" returned by open()
                sensor_data = "" # Initialize sensor data varible

                while (sensor_data != "END\n"): # Collect sensor data until "END" recieved
                        sensor_data = file_object.readline()   # Grab a line from sensor file, readline expects message to end with "\n"
                        conn.send(sensor_data.encode()) # Send a line of sensor data to client. Convert from String to bytes (i.e., encode) 
                        print(sensor_data) # Pring line of Sensor data

                file_object.close() # Important to close file once you are done with it!!                

        else:                
                # Convert message from bytes to String (i.e., decode), then print
                print("Recieved from client: " + data.decode()) 

                data_to_client = data.decode()  # Echo Server: option for echoing data back to client, or 
                #data_to_client = input('Enter data to sent to the client -> ') + '\n'  # Chat Server: option for interactively choosing what data to send to the client
        
                # Convert String to bytes (i.e., encode), and send data to the client
                conn.send(data_to_client.encode()) 
                print("Sent to client:" + data_to_client)

print("Server exiting\n")
time.sleep(3) # Sleep for 3 seconds
conn.close()  # close the Socket connection
