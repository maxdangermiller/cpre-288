import numpy as np
import matplotlib.pyplot as plt
import os
import sys

from clean_data import clean_file_data

if len(sys.argv) < 2:
	print("Missing REQUIRED arguments!\n\nFormat\n\tpython plot_clean.py [text-filename:str] [threshold(optional):int]\n")

THRESHOLD = 20
if len(sys.argv) >= 3:
	THRESHOLD = int(sys.argv[2])

absolute_path = os.path.dirname(__file__)
relative_path = "./"
full_path = os.path.join(absolute_path, relative_path)
filename = sys.argv[1] #

angle_degrees = []
angle_radians = []
distance = []
c_angle_degrees = []
c_angle_radians = []
c_distance = []

file_object = open(full_path + filename,'r')
file_header = file_object.readline()
file_data = file_object.readlines()
file_object.close()

c_file_data = clean_file_data(file_data, THRESHOLD)
c_file_data.pop(0) # Remove header

# For each line of the file split into columns, and assign each column to a variable
for line in file_data: 
	data = line.split()
	angle_degrees.append(float(data[0]))
	distance.append(float(data[1]) / 100)

for line in c_file_data: 
	data = line.split()
	c_angle_degrees.append(float(data[0]))
	c_distance.append(float(data[1]) / 100) 

angle_degrees = np.array(angle_degrees)
angle_radians = (np.pi/180) * angle_degrees

c_angle_degrees = np.array(c_angle_degrees)
c_angle_radians = (np.pi/180) * c_angle_degrees

# Create a polar plot
fig, ax = plt.subplots(subplot_kw={'projection': 'polar'})
ax.plot(angle_radians, distance, color='r', linewidth=2.0)
ax.plot(c_angle_radians, c_distance, color='b', linewidth=2.0)

ax.set_xlabel('Distance (mm)', fontsize = 14.0)             # Label x axis
ax.set_ylabel('Angle (degrees)', fontsize = 14.0)           # Label y axis
ax.xaxis.set_label_coords(0.5, 0.15)                        # Modify location of x axis label (Typically do not need or want this)
ax.tick_params(axis='both', which='major', labelsize=14)    # set font size of tick labels
ax.set_rmax(2.0)                        # type: ignore      # Saturate distance at 2.5 meters
ax.set_rticks([0.5, 1, 1.5, 2, 2.5])    # type: ignore 		# Set plot "distance" tick marks at .5, 1, 1.5, 2, and 2.5 meters
ax.set_rlabel_position(-22.5)     		# type: ignore 		# Adjust location of the radial labels
ax.set_thetamax(180)              		# type: ignore 		# Saturate angle to 180 degrees
ax.set_xticks(np.arange(0,np.pi+.1,np.pi/4)) 				# Set plot "angle" tick marks to pi/4 radians 
															# 	(i.e., displayed at 45 degree) increments
											 				# Note: added .1 to pi to go just beyond pi 
                											# 	(i.e., 180 degrees) so that 180 degrees is displayed
ax.grid(True)                     							# Show grid lines

# Create title for plot (font size = 14pt, y & pad controls title vertical location)
ax.set_title(f"CyBot Sensor Scan - {filename}", size=14, y=1.0, pad=-24) 
plt.show()  # Display plot
