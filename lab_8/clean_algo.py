def clean(angles:list[int], s_dists:list[float], ir_dists:list[float]):
	# Step 1: Elimantie all data over 1m
	for i in range(len(s_dists)):
		if s_dists[i] > 100:
			s_dists[i] = 100
	
	for i in range(len(ir_dists)):
		if ir_dists[i] > 100:
			ir_dists[i] = 100


	fix_outliers(s_dists)
	fix_outliers(ir_dists)

	smooth(s_dists)
	smooth(ir_dists)

	smooth(ir_dists, threshold=10)

	


def fix_outliers(arr:list[float], threshold = 2) -> list[str]:
	for i in range(1, len(arr) - 1):
		pre = arr[i - 1]
		cur = arr[i]
		nxt = arr[i + 1]

		pre_dt = cur - pre
		nxt_dt = cur - nxt

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