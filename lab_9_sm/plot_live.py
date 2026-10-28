# Description: Display using a polar plot the distance measurements collected by a CyBot 180 degree 
# sensor scan at 4 degree increments from 0 to 180 degrees. Sensor data read from a text file.

# Original Code example modified by: Phillip Jones (10/02/2021), (05/15/2023)
# Original polar plot code example from matplot: https://matplotlib.org/stable/gallery/pie_and_polar_charts/polar_demo.html

# Useful matplotlib tutorial: https://matplotlib.org/stable/tutorials/introductory/pyplot.html
# Useful best practices Quick Start: https://matplotlib.org/stable/tutorials/introductory/quick_start.html
# General Python Reference/Tutorials: https://www.w3schools.com/python/ 

# Quick YouTube Overviews (See above links as primary resources for additional details): 
# - Quick Polar Plot (subplot) Overview: https://www.youtube.com/watch?v=pb-pZtvkGPM
# - Quick subplots Overview : https://www.youtube.com/watch?v=Tqph7_qMujk

#Import/Include useful math and plotting functions
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import os  # import function for finding absolute path to this python script
import sys

absolute_path = os.path.dirname(__file__)
relative_path = "./logs/"
full_path = os.path.join(absolute_path, relative_path)
filename = "putty.txt"

# Create a polar plot
fig, ax = plt.subplots(subplot_kw={'projection': 'polar'}, figsize=(18, 10))

def animate(frame):

	angle_degrees = []
	angle_radians = []
	s_dists = []
	ir_dists = []
	clean_s_dists = []
	clean_ir_dists = []

	trig_degs = []
	trig_rads = []

	obj_degs = []
	obj_rads = []
	obj_dists = []
	obj_sizes = []
	
	try:
		with open(full_path + filename, 'r') as f:
			lines = f.read().splitlines()

			rawStart = -1
			cleanStart = -1
			trigStart = -1
			trigEnd = -1
			objStart = -1
			objEnd = -1

			for i in reversed(range(len(lines))):
				line = lines[i]
				# print(line)

				if line == "** Cleaned data **":
					cleanStart = i

				elif line == "**Raw noisy sensor data**":
					rawStart = i
					break

				elif line == "** Cleaned data (after trig) **":
					trigStart = i

				elif line[:7] == "Object#":
					trigEnd = i - 1
					objStart = i + 1
				
				elif line[:8] == "Smallest":
					objEnd = i


			# print(f"Start: {lines[start]}")
			# print(f"End: {lines[end - 1]}")
			
			for i in range(rawStart + 2, cleanStart):
				data = lines[i].split()
				angle_degrees.append(float(data[0]))
				s_dists.append(float(data[1]))
				ir_dists.append(float(data[2]))

			angle_degrees = np.array(angle_degrees)
			angle_radians = (np.pi/180) * angle_degrees

			for i in range(cleanStart + 2, trigStart):
				data = lines[i].split()
				clean_s_dists.append(float(data[1]))
				clean_ir_dists.append(float(data[2]))


			"""
			for i in range(trigStart + 2, trigEnd):
				data = lines[i].split()
				trig_degs.append(float(data[0]))
				trig_dists.append(float(data[1]))
			
			trig_degs = np.array(trig_degs)
			trig_rads = (np.pi/180) * trig_degs
			"""

			new_rads = angle_radians - (np.pi / 2.0)

			# trig_rads = np.atan(np.tan(new_rads) + 10.0 / (clean_s_dists * np.cos(new_rads))) + np.pi / 2.0
			trig_rads = np.atan((clean_s_dists * np.sin(new_rads)) / (clean_s_dists * np.cos(new_rads) + 10.0)) + (np.pi / 2.0)
			trig_degs = trig_rads * (180 / np.pi)

			for i in range(objStart, objEnd):
				data = lines[i].replace('\t', ' ').split()
				print(data)
				obj_degs.append(float(data[1]))
				obj_dists.append(float(data[2]))
				obj_sizes.append(float(data[4]))


			obj_degs = np.array(obj_degs)
			obj_rads = (np.pi/180) * obj_degs
	except:
		print("File doesn't have data yet.")
		return

	ax.clear()

	if "--raw" in sys.argv:
		ax.plot(angle_radians, s_dists, color='#0000FF', linewidth=1.0, label='Sound Distance')
		ax.plot(angle_radians, ir_dists, color='#FF0000', linewidth=1.0, label='IR Distance')

	if "--clean" in sys.argv:
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
	ax.set_title(f"Live output from PuTTy Log File", size=14, y=1.0, pad=-24) 

ax.set_xlabel('Distance (cm)', fontsize = 14.0)
ax.set_ylabel('Angle (degrees)', fontsize = 14.0)
ax.xaxis.set_label_coords(0.5, 0.15)
ax.set_rticks(range(0, 200, 10))
ax.set_rmax(200)
ax.set_rlabel_position(-22.5)
ax.set_thetamax(180)
ax.set_xticks(np.arange(0,np.pi+.1,np.pi/4))

ani = animation.FuncAnimation(fig, animate, interval=2000)

plt.show()  # Display plot