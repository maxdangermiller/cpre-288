/**
 * @name Handle Data
 * @headerfile handle_data.h
 * @author Max Miller
 */
#include "handle_data.h"

static void smooth(SensorData_t sensor_data[], int len, int threshold);

/**
 * Clean Data
 * @brief Cleans sensor data array by using edge-averages
 * @param sensor_data array of SensorData
 * @param len length of sensor_data array
 */
void clean_data(SensorData_t sensor_data[], int len) {

	int i = 0;

	// Step 1: Elimantie all data over 1m
	for (i = 0; i < len; i++) {
		if (sensor_data[i].s_dist > 125.0) {
			sensor_data[i].s_dist = 125.0;
		}
		if (sensor_data[i].ir_dist > 125.0) {
			sensor_data[i].ir_dist = 125.0;
		}
	}


	// fix_outliers(sensor_data, len);

	smooth(sensor_data, len, 10);

	smooth(sensor_data, len, 10);
}
	
int outlier_ratio(float one, float two) {
	if (one > 40.0 && two > 40.0) {
		return 1;
	}

	float val = fabsf(one / two);

	return 0.67 < val && val < 1.5;
}

/**
 * Fix Outliers
 */
void fix_outliers(SensorData_t sensor_data[], int len) {
	int i;
	SensorData_t pre, cur, nxt;
	float pre_dt_s, pre_dt_ir, nxt_dt_s, nxt_dt_ir;

	float avg_s, avg_ir;

	for (i = 1; i < len - 1; i++) {

		pre = sensor_data[i - 1];
		cur = sensor_data[i];
		nxt = sensor_data[i + 1];
		
		pre_dt_s = cur.s_dist - pre.s_dist;
		pre_dt_ir = cur.ir_dist - pre.ir_dist;

		nxt_dt_s = cur.s_dist - nxt.s_dist;
		nxt_dt_ir = cur.ir_dist - nxt.ir_dist;
		
		avg_s = (pre.s_dist + nxt.s_dist) / 2.0;
		avg_ir = (pre.ir_dist + nxt.ir_dist) / 2.0;

		if (outlier_ratio(pre_dt_s, nxt_dt_s)) {
			if (pre_dt_s > OUTLIER_THRESHOLD && nxt_dt_s > OUTLIER_THRESHOLD) {
				sensor_data[i].s_dist = avg_s;
			}
			else if (pre_dt_s < -OUTLIER_THRESHOLD && nxt_dt_s < -OUTLIER_THRESHOLD) {
				sensor_data[i].s_dist = avg_s;
			}
		}
		
		if (outlier_ratio(pre_dt_ir, nxt_dt_ir)) {

			if (pre_dt_ir > OUTLIER_THRESHOLD && nxt_dt_ir > OUTLIER_THRESHOLD) {
				sensor_data[i].ir_dist = avg_ir;
			}
			
	
			if (pre_dt_ir < -OUTLIER_THRESHOLD && nxt_dt_ir < -OUTLIER_THRESHOLD) {
				sensor_data[i].ir_dist = avg_ir;
			}
		}

	}
}





/**
 * Smooth
 */
static void smooth(SensorData_t sensor_data[], int len, int threshold) {

	int start = DATA_START_POS;
	int i, j;

	float delta;
	float tot, avg;
	
	for (i = start + 1; i < DATA_END_POS; i++) {

		delta = fabsf(sensor_data[i].s_dist - sensor_data[start].s_dist);
		// delta_ir = fabsf(sensor_data[i].ir_dist - sensor_data[start].ir_dist);
		
		if (delta > threshold || i == DATA_END_POS - 1) {
			tot = 0.0;
			for (j = start; j < i; j++) {
				tot += sensor_data[j].s_dist;
			}
				
			avg = tot / (float)(i - start);
				
			for (j = start; j < i; j++) {
				sensor_data[j].s_dist = avg;
			}
			
			start = i;
		}
		i++;
	}

	start = DATA_START_POS;

	for (i = start + 1; i < DATA_END_POS; i++) {

		delta = fabsf(sensor_data[i].ir_dist - sensor_data[start].ir_dist);
		
		if (delta > threshold || i == DATA_END_POS - 1) {
			tot = 0.0;
			for (j = start; j < i; j++) {
				tot += sensor_data[j].ir_dist;
			}
				
			avg = tot / (float)(i - start);
				
			for (j = start; j < i; j++) {
				sensor_data[j].ir_dist = avg;
			}
			
			start = i;
		}
		i++;
	}
}
	
/**
 * Approximate Sizes
 * @param sensor_data
 * @param len length
 * @param objects
 * @returns objects found
 */
int find_objects(SensorData_t sensor_data[], int len, ObjectInfo_t* objects) {
	// Find edges

	int i;

	ObjectInfo_t object;
	SensorDataSection_t section;

	int obj_index = 0;
	
	int start = DATA_START_POS;

	for (i = DATA_START_POS + 1; i < DATA_END_POS; i++) {
		if (sensor_data[i].ir_dist != sensor_data[i-1].ir_dist) {
			section.start_ang = sensor_data[start].ang;
			section.end_ang = sensor_data[i - 1].ang;
			section.s_dist = sensor_data[start].s_dist;
			section.ir_dist = sensor_data[start].ir_dist;

			if (
				sensor_data[start - 1].ir_dist > section.ir_dist && 
				sensor_data[i].ir_dist > section.ir_dist &&
				section.s_dist < 100.0
			) {
				if (identify_object(&object, &section)) {
					object.num = obj_index + 1;
					objects[obj_index++] = object;
				}
			}

			start = i;
		}
	}


	section.start_ang = sensor_data[start].ang;
	section.end_ang = sensor_data[len - 1].ang;
	section.s_dist = sensor_data[start].s_dist;
	section.ir_dist = sensor_data[start].ir_dist;

	// Don't count the last section as an object
	/*
	if (identify_object(&object, &section)) {
		object.num = obj_index + 1;
		objects[obj_index++] = object;
	}
	*/

	return obj_index;
}


/**
 * Convert Sensor Angle to Bot Angle
 * @param a_s angle in degrees
 * @param d_s distance in cm
 */
static double convert_s_ang(double a_s, double d_s) {
	a_s = (a_s - 90.0) * (M_PI / 180.0);
	d_s = d_s + 6.0;
	// np.atan((clean_s_dists * np.sin(new_rads)) / (clean_s_dists * np.cos(new_rads) + 10.0)) + (np.pi / 2.0)
	double corrected = atan((d_s * sin(a_s)) / (d_s * cos(a_s) + 10.0)) + (M_PI / 2.0);
	return corrected * 180.0 / M_PI;

}


/**
 * Identify Object
 * @brief Identify if object is in the data section
 * @param object is a reference to object info to write to if this is a valid object
 * @param data_section is a reference to the data section to analyze
 */
int identify_object(ObjectInfo_t *object, SensorDataSection_t *data_section) {
	if (data_section->s_dist >=  100) {
		return 0;
	}

	const int ang_width = data_section->end_ang - data_section->start_ang;

	object->mp = (float)ang_width / 2.0 + data_section->start_ang;

	//recalculate midpoint using trig 
	object->mp = convert_s_ang(object->mp, data_section->s_dist);
	
	object->s_dist = data_section->s_dist;
	object->ir_dist = data_section->ir_dist;
	object->len = get_linear_size(data_section);

	if (object->len < 1.0 || object->len >= 30.0) {
		return 0;
	}

	return 1;
}


/**
 * Get Linear Size
 * @param data_section SensorDataSection to find the size of it useing the angles and linear size
 */
float get_linear_size(SensorDataSection_t *data_section) {
	const float a = data_section->s_dist;
	const float rad_width = (float)(data_section->end_ang - data_section->start_ang) / 360.0 * 2.0 * M_PI;

	// a^2 + b^2 - 2 * a * b * cos(ang C)
	return sqrtf((2 * a * a) - (2 * a * a * cosf(rad_width)));
}


/**
 * @param objects
 * @param len length
 * @returns index in objects of smallest object
 */
int get_smallest_obj(ObjectInfo_t *objects, int len) {
	int min = 0;
	int i;

	for (i = 1; i < len; i++) {
		if (objects[i].len < objects[min].len) {
			min = i;
		}
	}

	return min;
}


/**
 * Send Object Table
 * @param objects
 * @param object_count
 */
void send_object_table(ObjectInfo_t *objects, int object_count) {
	char str[50];
	int i;
	ObjectInfo_t object;

	sprintf(str, "\r\nObject#\tAngle (deg)\tSound Dist (cm)\tIR Dist (cm)\tWidth (cm)\r\n");
	uart_sendStr(str);

	for(i = 0; i < object_count; i++) {
		object = objects[i];
		sprintf(str, "%d\t\t%.1f\t\t%.2f\t\t%.2f\t\t%.2f\r\n", object.num, object.mp, object.s_dist, object.ir_dist, object.len);
		uart_sendStr(str);
	}
}
