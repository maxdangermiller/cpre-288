def clean(angles:list[int], s_dists:list[float], ir_dists:list[float]):
	# Step 1: Elimantie all data over 1.25m
	for i in range(len(s_dists)):
		if s_dists[i] > 125:
			s_dists[i] = 125

	for i in range(len(ir_dists)):
		if ir_dists[i] > 200:
			ir_dists[i] = 200


	# fix_outliers(s_dists)
	# fix_outliers(ir_dists)

	# smooth(s_dists)
	smooth_ir(ir_dists)

	


def fix_outliers(arr:list[float], threshold = 2) -> list[str]:
	for i in range(1, len(arr) - 1):
		pre = arr[i - 1]
		cur = arr[i]
		nxt = arr[i + 1]

		pre_dt = cur - pre
		nxt_dt = cur - nxt

		print(f"Evaluation point {i * 2}, pre_dt = {pre_dt} and nxt_dt = {nxt_dt}")

		avg = (pre + nxt) / 2
		
		if (pre_dt > threshold and nxt_dt > threshold):
			arr[i] = avg
			continue

		if (pre_dt < -threshold and nxt_dt < -threshold):
			arr[i] = avg
			continue
	
def smooth(arr:list[float], start = 0, threshold = 10):
	i = start + 1

	while i < len(arr):
		delta = abs(arr[i] - arr[start])

		if (delta > threshold) or (i == len(arr) - 1):
			print(f"Writing range {start} to {i}", end="")
			
			tot = 0.0
			for j in range(start, i):
				tot += arr[j]

			avg = tot / (i - start)

			
			print(f".  Avg: {avg}")
	
			for j in range(start, i):
				arr[j] = avg
			
			start = i
		i += 1

def smooth_ir(sensor_data:list[float], threshold = 10):

	start = 1

	for i in range(start + 1, 90):
		delta = abs(sensor_data[i] - sensor_data[start])
		
		if ((delta > threshold and i - start > 1) or i == 89):
			tot = 0.0
			print(f"Writing range {start} to {i}", end="")

			for j in range(start, i):
				tot += sensor_data[j]
				
			avg = tot / (float)(i - start)

			print(f".  Avg: {avg}")
				
			for j in range(start, i):
				sensor_data[j] = avg
			
			
			start = i

def ir_part_2(sensor_data:list[float], threshold = 10):
	
	print("\n\nDOING SECOND IR SMOOTHING\n\n")

	# fix_outliers(sensor_data)

	
	start = 1
	had_last_delta = False
	last_delta = 0
	count = 100

	# This time make sure each section is not going the same direction
	for i in range(start + 1, 90):

		delta = sensor_data[i] - sensor_data[i - 1]

		if abs(delta) < 0.5:
			# do nothing
			pass
		elif not had_last_delta:
			had_last_delta = True
			last_delta = delta
			start = i
			print(f"\tFound first delta of {delta}")

		elif ((delta * last_delta) < 0):
			print(f"\tOppisite directions! {delta}/{last_delta} from {i}/{start}")
			print(f"\t\tWriting range {start} to {i}", end="")

			if (delta < last_delta):
				count -= 20

			else:
				count += 20

			min = 200.0
			min_start = -1
			min_end = -1

			for j in range(start, i):
				if sensor_data[j] < min:
					min_start = j
					min = sensor_data[j]
				elif abs(sensor_data[j] - min) < 0.1:
					min_end = j
				
			print(f".  Min: {min}")
				
			# for j in range(start, min_start):
			# 	sensor_data[j] = 200

			for j in range(start, i):
				sensor_data[j] = min

			# for j in range(min_end + 1, i):
			# 	sensor_data[j] = 200

			
			
			start = i
			last_delta = delta
		else:
			print(f"\tFound delta in the same direction: {delta}/{last_delta}")
