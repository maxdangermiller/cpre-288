import socket
import threading
from collections import deque
import numpy as np
import sys


class CybotUART:
	def __init__(self, host, port):

		self.HOST = host
		self.PORT = port

		self.map_listen = False
		self.auto_mode = False
		self.data_stack = deque()

	def receive_messages(self):
		pending = ""
		
		"""Background task that waits for and handles new server messages."""
		while True:
			try:
				raw_data = self.cybot_socket.recv(1028)
				if not raw_data:
					print("\nDisconnected from server.")
					break

				data = raw_data.decode()
				lines = data.splitlines()

				if len(lines) == 0:
					pending = data
					continue
				
				# If there's something pending, go ahead and write it with the remainer of the line
				if len(pending) > 0:
					# print("Wrote Pending.")
					self.data_stack.append(pending + lines[0].replace('\t', ' '))
					print(self.data_stack[-1])
					pending = ""

				# Otherwise if there are more than 1 line, go ahead and write the first one
				elif len(lines) > 1:
					self.data_stack.append(lines[0].replace('\t', ' '))
					print(self.data_stack[-1])
				

				# If there are more than 2 lines, write all except the last one
				for line in lines[1:-1]:
					self.data_stack.append(line.replace('\t', ' '))
					print(self.data_stack[-1])

				# If the last line is complete, write it
				if data[len(data) - 1] == '\n':
					self.data_stack.append(lines[len(lines) - 1].replace('\t', ' '))
					print(self.data_stack[-1])
				
				# Otherwise if it's not complete set the line as pending
				else:
					pending = lines[len(lines) - 1].replace('\t', ' ')

			except (ConnectionResetError, OSError):
				break

	def connect(self):
		self.cybot_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.cybot_socket.connect((self.HOST, self.PORT))

		self.listener_thread = threading.Thread(target=self.receive_messages)
		self.listener_thread.daemon = True  # Allows thread to exit when main program closes
		self.listener_thread.start()

	def close(self):
		if self.cybot_socket:
			self.cybot_socket.close()

	def send(self, cmd:str):

		for c in cmd:
			self.handle_cmd(c)
			self.cybot_socket.send(c.encode())	

	def handle_cmd(self, cmd:str):

		if cmd == 'm' or cmd == 'z':
			self.map_listen = True

		elif cmd == 'h' and self.auto_mode:
			self.map_listen = True

		elif cmd == 't':
			self.auto_mode = not self.auto_mode

	def isAutoMode(self) -> bool:
		return self.auto_mode

	def shouldMapListen(self) -> bool:
		return self.map_listen

	def getDataStack(self) -> deque:
		return self.data_stack


if __name__ == "__main__":

	cybot = CybotUART("192.168.1.1", 288)
	cybot.connect()

	print("Ready to send messages. Type your text and press Enter.")

	while True:
		try:
			user_in = input("> ")
			if user_in.lower() == 'exit':
				break

			for cmd in user_in:
				cybot.send(cmd)
			
		except (KeyboardInterrupt, BrokenPipeError):
			break

	cybot.close()
	print("Client shutdown cleanly.")
