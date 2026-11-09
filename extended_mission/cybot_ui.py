# Designed with https://visualtkinter.com - paste this file into File > Import there to edit it visually
import tkinter as tk
from tkinter import ttk
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
from cybot_uart import CybotUART


class Application:
	def __init__(self, root):
		# Configure UI Root
		self.root = root
		root.title("Main")
		root.geometry("1000x700")
		root.update_idletasks()
		root.geometry(
			f"+{(root.winfo_screenwidth() - 1000) // 2}"
			f"+{(root.winfo_screenheight() - 700) // 2}"
		)

		self.cybot = CybotUART("192.168.1.1", 288)
		self.cybot.connect()

		self.title = tk.Label(root, text="SE8 Extended Mission", font=("Helvetica", 14, "bold"))
		self.title.place(x=300, y=10, width=400, height=30)

		self.modeBtn = tk.Button(root, text="Toggle Mode", command=self.toggleMode)
		self.modeBtn.place(x=85, y=630, width=170, height=30)

		self.modeState = tk.StringVar()
		self.modeState.set("Current Mode: MANUAL")

		self.modeText = tk.Label(root, textvariable=self.modeState)
		self.modeText.place(x=90, y=600, width=160, height=30)

		self.objectListTxt = tk.StringVar()
		self.objectListTxt.set("")

		self.objectList = tk.Label(root, textvariable=self.objectListTxt)
		self.objectList.place(x=455, y=615, width=400, height=100)

		# Configure plot
		self.dataPlot = FigureCanvasTkAgg(Figure(figsize=(6.50, 5.50), dpi=100, layout="constrained"), master=root)
		self.ax = self.dataPlot.figure.add_subplot(111, projection="polar")
		self.ax.set_title("Chart")
		self.ax.set_xlabel('Distance (cm)', fontsize = 14.0)
		self.ax.set_ylabel('Angle (degrees)', fontsize = 14.0)
		self.ax.xaxis.set_label_coords(0.5, 0.15)

		self.ax.set_rticks(range(0, 220, 20))
		self.ax.set_rmax(200)
		self.ax.set_rlabel_position(-22.5)
		self.ax.set_thetamax(180)

		self.ax.set_xticks(np.arange(0,np.pi+.1,np.pi/4))
		self.ax.xaxis.set_label_coords(0.5, 0.15)
		
		self.ax.grid(True)
		self.ax.set_title("Live output", size=14, y=1.0, pad=-24) 

		self.s_plt = self.ax.plot([], [], color='#0000FF', linewidth=1.0, label='Sound Distance')
		self.ir_plt = self.ax.plot([], [], color='#FF0000', linewidth=1.0, label='IR Distance')
	
		self.cs_plt = self.ax.plot([], [], color='#00FF00', linewidth=3.0, label='Cleaned Sound Distance')
		self.cir_plt = self.ax.plot([], [], color="#00A29A", linewidth=3.0, label='Cleaned IR Distance')

		self.dataPlot.draw()
		self.dataPlot.get_tk_widget().place(x=330, y=130, width=650, height=550)

		self.root.after(1000, self.update_plot)

		self.forwardBtn = tk.Button(root, text="", command=self.on_forwardBtn)
		self.forwardBtn.place(x=140, y=180, width=60, height=60)
		root.bind("w", self.on_forwardBtn)

		self.leftBtn = tk.Button(root, text="", command=self.on_leftBtn)
		self.leftBtn.place(x=60, y=260, width=60, height=60)
		root.bind("a", self.on_leftBtn)

		self.rightBtn = tk.Button(root, text="", command=self.on_rightBtn)
		self.rightBtn.place(x=220, y=260, width=60, height=60)
		root.bind("d", self.on_rightBtn)

		self.backBtn = tk.Button(root, text="", command=self.on_backBtn)
		self.backBtn.place(x=140, y=340, width=60, height=60)
		root.bind("s", self.on_backBtn)

		self.mapBtn = tk.Button(root, text="Map", command=self.on_mapBtn)
		self.mapBtn.place(x=35, y=430, width=110, height=40)
		root.bind("m", self.on_mapBtn)

		self.posBtn = tk.Button(root, text="Q", command=self.on_posBtn)
		self.posBtn.place(x=140, y=260, width=60, height=60)

		self.homeBtn = tk.Button(root, text="Return Home", command=self.on_homeBtn)
		self.homeBtn.place(x=195, y=430, width=110, height=40)

		self.objMoveBtn = tk.Button(root, text="Move To Object", command=self.on_objMoveBtn)
		self.objMoveBtn.place(x=115, y=490, width=110, height=40)

	def on_forwardBtn(self, *args):
		self.cybot.send('w')

	def on_leftBtn(self, *args):
		self.cybot.send('a')

	def on_rightBtn(self, *args):
		self.cybot.send('d')

	def on_backBtn(self, *args):
		self.cybot.send('s')

	def on_mapBtn(self, *args):
		self.cybot.send('m')

	def on_posBtn(self):
		self.cybot.send('q')

	def on_homeBtn(self):
		self.cybot.send('r')

	def on_objMoveBtn(self):
		self.cybot.send('c')

	def toggleMode(self):
		if self.cybot.isAutoMode():
			self.modeState.set("Current Mode: MANUAL")
		else:
			self.modeState.set("Current Mode: AUTO")
		self.cybot.send('t')

	def update_plot(self):
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

		data_stack = self.cybot.getDataStack()
	
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
	
	
		if rawStart == -1 or cleanStart == -1 or trigStart == -1 or objStart == -1 or objEnd == -1:
			self.root.after(1000, self.update_plot)
			return
		
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
	
		trig_rads = np.atan((clean_s_dists * np.sin(new_rads)) / (clean_s_dists * np.cos(new_rads) + 10.0)) + (np.pi / 2.0)
		
		for i in range(objStart, objEnd):
			data = data_stack[i].replace('\t', ' ').split()
			obj_degs.append(float(data[1]))
			obj_dists.append(float(data[2]))
			obj_sizes.append(float(data[4]))

		print(list(data_stack)[objStart:objEnd])

		self.objectListTxt.set("\n".join(list(data_stack)[objStart-1:objEnd+1]))
		self.objectList.tkraise()
	
		data_stack.clear()
	
	
		obj_degs = np.array(obj_degs)
		obj_rads = (np.pi/180) * obj_degs
	
		self.ax.clear()

		# self.s_plt.set_data(angle_radians, s_dists)
		# self.ir_plt.set_data(angle_radians, ir_dists)

		# self.cs_plt.set_data(angle_radians, clean_s_dists)
		# self.cir_plt.set_data(angle_radians, clean_ir_dists)

		self.ax.plot(angle_radians, s_dists, color='#0000FF', linewidth=1.0, label='Sound Distance', zorder=1)
		self.ax.plot(angle_radians, ir_dists, color='#FF0000', linewidth=1.0, label='IR Distance', zorder=2)
	
		self.ax.plot(angle_radians, clean_s_dists, color='#00FF00', linewidth=3.0, label='Cleaned Sound Distance', zorder=3)
		self.ax.plot(angle_radians, clean_ir_dists, color="#00A29A", linewidth=3.0, label='Cleaned IR Distance', zorder=4)

		# ax.plot(trig_rads, clean_s_dists, color="#C00ED4", linewidth=1.0, label='Trig Sound Distance')
		# ax.plot(trig_rads, clean_ir_dists, color="#D40E5D", linewidth=1.0, label='Trig IR Distance')
	
		for i in range(len(obj_rads)):
			self.ax.scatter(obj_rads[i], obj_dists[i], color="#000000", linewidth=obj_sizes[i], label=f'Object #{i + 1}', zorder=10)

		self.dataPlot.figure.legend()

		self.ax.set_rmax(200)
		self.ax.set_thetamax(180)

		"""
			
		self.ax.set_xlabel('Distance (cm)', fontsize = 14.0)
		self.ax.set_ylabel('Angle (degrees)', fontsize = 14.0)
		self.ax.xaxis.set_label_coords(0.5, 0.15)
		self.ax.set_rticks(range(0, 200, 10))
		self.ax.set_rmax(200)
		self.ax.set_rlabel_position(-22.5)
		self.ax.set_thetamax(180)
		self.ax.set_xticks(np.arange(0,np.pi+.1,np.pi/4))
	
		"""

		self.dataPlot.draw()
		
		self.root.after(1000, self.update_plot)


if __name__ == "__main__":
	root = tk.Tk()
	app = Application(root)
	root.mainloop()
