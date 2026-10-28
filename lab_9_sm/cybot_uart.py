import socket
import threading
import time
from collections import deque
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import os
import sys

map_listen = False
auto_mode = False

data_stack = deque()

HOST = '192.168.1.1'
PORT = 288

cybot_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
cybot_socket.connect((HOST, PORT))

# Set up plot
fig, ax = plt.subplots(subplot_kw={'projection': 'polar'}, figsize=(18, 10))

ax.set_xlabel('Distance (cm)', fontsize = 14.0)
ax.set_ylabel('Angle (degrees)', fontsize = 14.0)
ax.xaxis.set_label_coords(0.5, 0.15)
ax.set_rticks(range(0, 200, 10))
ax.set_rmax(200)
ax.set_rlabel_position(-22.5)
ax.set_thetamax(180)
ax.set_xticks(np.arange(0,np.pi+.1,np.pi/4))

def receive_messages(sock:socket.socket, data_stack:deque):
	pending = ""
	
	"""Background task that waits for and handles new server messages."""
	while True:
		try:
			raw_data = sock.recv(1028)
			if not raw_data:
				print("\nDisconnected from server.")
				break

			data = raw_data.decode()
			lines = data.splitlines()

			# print(f"\n{data}", end="")

			if len(lines) == 0:
				pending = data
				# print(f"Marked \"{pending}\" as pending (2)!")
				continue
			

			if len(pending) > 0:
				# print("Wrote Pending.")
				data_stack.append(pending + lines[0].replace('\t', ' '))
				print(data_stack[-1])
				pending = ""
			elif len(lines) > 1:
				data_stack.append(lines[0].replace('\t', ' '))
				print(data_stack[-1])
			

			for line in lines[1:-1]:
				data_stack.append(line.replace('\t', ' '))
				print(data_stack[-1])

			if data[len(data) - 1] == '\n':
				data_stack.append(lines[len(lines) - 1].replace('\t', ' '))
				print(data_stack[-1])
				# print(f"Complete Data frame: \"{line.replace('\t', ' ')}\"")
			else:
				pending = lines[len(lines) - 1].replace('\t', ' ')
				# print(f"Marked \"{pending}\" as pending!")

			# print(f"Stack Size: {len(data_stack)}, last element: {data_stack[len(data_stack) - 1]}")
			
			# print("\r> ", end="") # Re-print user prompt
		except (ConnectionResetError, OSError):
			break

# 1. Start a daemon thread to listen for incoming messages
listener_thread = threading.Thread(target=receive_messages, args=(cybot_socket, data_stack))
listener_thread.daemon = True  # Allows thread to exit when main program closes
listener_thread.start()

def handle_cmd(cmd:str):
	global auto_mode
	global map_listen

	if cmd == 'm' or cmd == 'z':
		map_listen = True

	elif cmd == 'h' and auto_mode:
		map_listen = True

	elif cmd == 't':
		auto_mode = not auto_mode

def graph(frame):
	global data_stack
	if not map_listen or len(data_stack) == 0:
		return

	angle_degrees = []
	angle_radians = []
	s_dists = []
	ir_dists = []
	clean_s_dists = []
	clean_ir_dists = []

	trig_rads = []

	obj_degs = []
	obj_rads = []
	obj_dists = []
	obj_sizes = []
	
	rawStart = -1
	cleanStart = -1
	trigStart = -1
	objStart = -1
	objEnd = -1

	print(f"Trying to graph Stack of size {len(data_stack)}")

	for i in reversed(range(len(data_stack))):
		line = data_stack[i]

		if line == "** Cleaned data **":
			cleanStart = i

		elif line == "**Raw noisy sensor data**":
			rawStart = i
			break

		elif line == "** Cleaned data (after trig) **":
			trigStart = i

		elif line[:7] == "Object#":
			objStart = i + 1
		
		elif line[:8] == "Smallest":
			objEnd = i

	print(rawStart, cleanStart, trigStart, objStart, objEnd)


	if rawStart == -1 or cleanStart == -1 or trigStart == -1 or objStart == -1 or objEnd == -1:
		return


	print(f"Start: {data_stack[rawStart]}")
	print(f"End: {data_stack[cleanStart]}")
	
	for i in range(rawStart + 2, cleanStart):
		data = data_stack[i].split()
		angle_degrees.append(float(data[0]))
		s_dists.append(float(data[1]))
		ir_dists.append(float(data[2]))

	angle_degrees = np.array(angle_degrees)
	angle_radians = (np.pi/180) * angle_degrees

	for i in range(cleanStart + 2, trigStart):
		data = data_stack[i].split()
		clean_s_dists.append(float(data[1]))
		clean_ir_dists.append(float(data[2]))

	new_rads = angle_radians - (np.pi / 2.0)

	# trig_rads = np.atan(np.tan(new_rads) + 10.0 / (clean_s_dists * np.cos(new_rads))) + np.pi / 2.0
	trig_rads = np.atan((clean_s_dists * np.sin(new_rads)) / (clean_s_dists * np.cos(new_rads) + 10.0)) + (np.pi / 2.0)

	for i in range(objStart, objEnd):
		data = data_stack[i].replace('\t', ' ').split()
		obj_degs.append(float(data[1]))
		obj_dists.append(float(data[2]))
		obj_sizes.append(float(data[4]))

	print(f"Length before clear: {len(data_stack)}")
	data_stack.clear()
	print(f"Length after clear: {len(data_stack)}")


	obj_degs = np.array(obj_degs)
	obj_rads = (np.pi/180) * obj_degs

	ax.clear()

	ax.plot(angle_radians, s_dists, color='#0000FF', linewidth=1.0, label='Sound Distance')
	ax.plot(angle_radians, ir_dists, color='#FF0000', linewidth=1.0, label='IR Distance')

	ax.plot(angle_radians, clean_s_dists, color='#00FF00', linewidth=3.0, label='Cleaned Sound Distance')
	ax.plot(angle_radians, clean_ir_dists, color="#00A29A", linewidth=3.0, label='Cleaned IR Distance')

	if "--trig" in sys.argv:
		# print(trig_degs)
		ax.plot(trig_rads, clean_s_dists, color="#C00ED4", linewidth=1.0, label='Trig Sound Distance')
		ax.plot(trig_rads, clean_ir_dists, color="#D40E5D", linewidth=1.0, label='Trig IR Distance')

	for i in range(len(obj_rads)):
		ax.scatter(obj_rads[i], obj_dists[i], color="#000000", linewidth=obj_sizes[i], label=f'Object #{i + 1}')

	ax.set_xlabel('Distance (cm)', fontsize = 14.0)
	ax.set_ylabel('Angle (degrees)', fontsize = 14.0)
	ax.xaxis.set_label_coords(0.5, 0.15)
	ax.set_rticks(range(0, 200, 10))
	ax.set_rmax(200)
	ax.set_rlabel_position(-22.5)
	ax.set_thetamax(180)
	ax.set_xticks(np.arange(0,np.pi+.1,np.pi/4))

	plt.legend()

	ax.grid(True)
	ax.set_title(f"Live output", size=14, y=1.0, pad=-24) 
	plt.draw()
	plt.pause(0.01)
	


# 2. Main thread stays free to wait for user typing and sending
if __name__ == "__main__":
	plt.ion()
	ani = animation.FuncAnimation(fig, graph, interval=2000)
	plt.show(block=False)

	print("Ready to send messages. Type your text and press Enter.")

	while True:
		try:
			user_in = input("> ")
			if user_in.lower() == 'exit':
				break

			for cmd in user_in:
				cybot_socket.send(cmd.encode())
				handle_cmd(cmd)
			
		except (KeyboardInterrupt, BrokenPipeError):
			break

		

	cybot_socket.close()
	print("Client shutdown cleanly.")
