import os
import sys

OUTLIER_THRESHOLD = 50

debug = __name__ != "__main__"


def fix_outliers(file_data: list[str], threshold = 20) -> list[str]:
	out = []
	out.append(file_data[0])
	
	for i in range(1, len(file_data) - 1):
		deg = int(file_data[i].split()[0])
		pre = float(file_data[i - 1].split()[1])
		cur = float(file_data[i].split()[1])
		nxt = float(file_data[i + 1].split()[1])

		pre_dt = cur - pre
		nxt_dt = cur - nxt

		avg = (pre + nxt) / 2
		
		if (pre_dt > threshold and nxt_dt > threshold):
			out.append(f"{deg}\t\t\t{avg}")
			continue

		if (pre_dt < -threshold and nxt_dt < -threshold):
			out.append(f"{deg}\t\t\t{avg}")
			continue
		
		out.append(f"{deg}\t\t\t{cur}")
  
	return out


def clean_file_data(file_data: list[str], threshold = 20) -> list[str]:
	output_lines = list()
	output_lines.append("Angle(Degrees)\tDistance(mm)")
 
	# file_data = fix_outliers(file_data, 50)
	
	start = 3  # Start at index 2 bc for some reason the CyBot's first few values are always weird
	
	for i in range(start + 1, len(file_data)):
		line = file_data[i]
		data = line.split()

		start_line = file_data[start]
		start_data = start_line.split()

		delta = abs(float(data[1]) - float(start_data[1]))

		if (delta > threshold) or (i == len(file_data) - 1):
			if debug:
				print(f"Writing range {file_data[start].split()[0]} to {file_data[i].split()[0]}", end="")
			
			tot = 0.0
			for j in range(start, i):
				tot += float(file_data[j].split()[1])

			avg = tot / (i - start)

			if debug:
				print(f".  Avg: {avg}")
	
			for j in range(start, i):
				deg = file_data[j].split()[0]
				output_lines.append(f"{deg}\t\t\t{avg:.2f}")
				# print(f"{deg}\t\t\t{avg:.2f}")
	
			start = i

	return output_lines


if __name__ == "__main__":
 
	if len(sys.argv) < 2:
		print("Missing REQUIRED arguments!\n\nFormat\n\tpython clean_data.py [text-filename:str] [threshold(optional):int]\n")
 
	THRESHOLD = 20
	if len(sys.argv) >= 3:
		THRESHOLD = int(sys.argv[2])
 
	absolute_path = os.path.dirname(__file__)
	relative_path = "./"
	full_path = os.path.join(absolute_path, relative_path)
	filename = sys.argv[1] # Name of sensor data file

	file_object = open(full_path + filename,'r')
	file_header = file_object.readline()
	file_data = file_object.readlines()
	file_object.close()


	for line in clean_file_data(file_data, THRESHOLD):
		print(line)



