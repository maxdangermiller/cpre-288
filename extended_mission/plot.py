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

from clean_algo import clean

absolute_path = os.path.dirname(__file__)
relative_path = "./logs/"
full_path = os.path.join(absolute_path, relative_path)
filename = sys.argv[1]

# Create a polar plot
fig, ax = plt.subplots(subplot_kw={'projection': 'polar'}, figsize=(18, 10))

def animate(frame):

	angle_degrees = []
	angle_radians = []
	s_dists = []
	ir_dists = []
	clean_s_dists = []
	
	try:
		with open(full_path + filename, 'r') as f:
			lines = f.read().splitlines()

			rawStart = -1
			cleanStart = -1
			cleanEnd = -1

			for i in reversed(range(len(lines))):
				line = lines[i]
				# print(line)

				if line == "** Cleaned data **":
					cleanStart = i

				elif line == "**Raw noisy sensor data**":
					rawStart = i + 2
					break

				elif line[:7] == "Object#":
					cleanEnd = i - 1

			# print(f"Start: {lines[start]}")
			# print(f"End: {lines[end - 1]}")

			for i in range(rawStart, cleanStart):
				data = lines[i].split()
				angle_degrees.append(float(data[0]))
				s_dists.append(float(data[1]))
				ir_dists.append(float(data[2]))

			angle_degrees = np.array(angle_degrees)
			angle_radians = (np.pi/180) * angle_degrees

			for i in range(cleanStart + 2, cleanEnd):
				data = lines[i].split()
				clean_s_dists.append(float(data[1]))
	except:
		print("File doesn't have data yet.")


	ax.clear()

	ax.scatter(angle_radians, s_dists, color='b', s=4.0, label='Sound Distance')
	# ax.plot(angle_radians, clean_s_dists, color='g', linewidth=3.0, label='Cleaned Distance')
	ax.scatter(angle_radians, ir_dists, color='r', s=4.0, label='IR Distance')

	ax.plot(angle_radians, s_dists, color='b', linewidth=1.0, label='Sound Distance')
	# ax.plot(angle_radians, clean_s_dists, color='g', linewidth=3.0, label='Cleaned Distance')
	ax.plot(angle_radians, ir_dists, color='r', linewidth=1.0, label='IR Distance')

	if "--clean" in sys.argv:
			clean(angle_radians, s_dists, ir_dists)

	ax.plot(angle_radians, s_dists, color='g', linewidth=3.0, label='Cleaned Sound Distance')
	ax.plot(angle_radians, ir_dists, color="#FF8000", linewidth=3.0, label='Cleaned IR Distance')
	
	ax.set_xlabel('Distance (cm)', fontsize = 14.0)
	ax.set_ylabel('Angle (degrees)', fontsize = 14.0)
	ax.xaxis.set_label_coords(0.5, 0.15)
	ax.set_rmax(35)
	ax.set_rticks(range(0, 200, 10))
	ax.set_rlabel_position(-22.5)
	ax.set_thetamax(180)
	ax.set_xticks(np.arange(0,np.pi+.1,np.pi/4))

	plt.legend()

	ax.grid(True)
	ax.set_title(f"Live output from PuTTy Log File", size=14, y=1.0, pad=-24) 

# ani = animation.FuncAnimation(fig, animate, interval=2000)
animate(1)

plt.show()  # Display plot