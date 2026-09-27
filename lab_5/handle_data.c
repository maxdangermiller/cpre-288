/**
 * @name Handle Data
 * @headerfile handle_data.h
 * @author Max Miller
 */
#include "handle_data.h"


/**
 * Clean Data
 * @brief Cleans sensor data array by using edge-averages
 * @param sensor_data array of SensorData
 * @param len length of sensor_data array
 */
void clean_data (SensorData_t sensor_data[], int len) {
	int i, j;

	int deg;
	float cur, pre, nxt, avg, tot;
	float delta, pre_dt, nxt_dt;
	
	// Start at index 3
	// For some reason the CyBot's first few values are always weird
	int start = 0;
	float start_value = sensor_data[start].dist;

	for (i = start + 1; i < len; i++) {

		
		deg = sensor_data[i].ang;
		pre = sensor_data[i - 1].dist;
		cur = sensor_data[i].dist;
		nxt = sensor_data[i + 1].dist;

		delta = fabsf(cur - start_value);


		if (delta > JUMP_THRESHOLD) {
			tot = 0.0;

			for (j = start; j < i; j++) {
				tot += sensor_data[j].dist;
			}

			avg = tot / (i - start);

			for (j = start; j < i; j++) {
				sensor_data[j].dist = avg;
			}

			start = i;
			start_value = cur;
		}
	}

	// Write the rest of the values
	tot = 0.0;

	for (i = start; i < len; i++) {
		tot += sensor_data[i].dist;
	}

	avg = tot / (i - start);

	for (j = start; j < i; j++) {
		sensor_data[j].dist = avg;
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


	int section_count = 1;
	int obj_index = 0;
	
	int start = 0;

	for (i = 1; i < len; i++) {
		if (sensor_data[i].dist != sensor_data[i-1].dist) {
			section.start_ang = sensor_data[start].ang;
			section.end_ang = sensor_data[i].ang;
			section.dist = sensor_data[start].dist;

			if (identify_object(&object, &section)) {
				object.num = obj_index + 1;
				objects[obj_index++] = object;
			}

			section_count++;
			start = i;
		}
	}


	section.start_ang = sensor_data[start].ang;
	section.end_ang = sensor_data[len - 1].ang;
	section.dist = sensor_data[start].dist;

	if (identify_object(&object, &section)) {
		object.num = obj_index + 1;
		objects[obj_index++] = object;
	}

	return obj_index;
}


/**
 * Identify Object
 * @brief Identify if object is in the data section
 * @param object is a reference to object info to write to if this is a valid object
 * @param data_section is a reference to the data section to analyze
 */
int identify_object(ObjectInfo_t *object, SensorDataSection_t *data_section) {
	if (data_section->dist >=  135) {
		return 0;
	}

	const int ang_width = data_section->end_ang - data_section->start_ang;

	object->mp = (float)ang_width / 2.0 + data_section->start_ang;
	object->dist = data_section->dist;
	object->len = get_linear_size(data_section);

	if (object->len < 5) {
		return 0;
	}

	return 1;
}


/**
 * Get Linear Size
 * @param data_section SensorDataSection to find the size of it useing the angles and linear size
 */
float get_linear_size(SensorDataSection_t *data_section) {
	const float a = data_section->dist;
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

	sprintf(str, "\r\nObject#\t\tAngle (deg)\t\tDistance (mm)\t\tWidth (mm)\r\n");
	cyBot_sendString(str);

	for(i = 0; i < object_count; i++) {
		object = objects[i];
		sprintf(str, "%d\t\t\t%.1f\t\t\t%.2f\t\t\t%.2f\r\n", object.num, object.mp, object.dist, object.len);
		cyBot_sendString(str);
	}
}


/**
 * cyBot send string back to user
 * @param str
 */
void cyBot_sendString(char* str) {
	int i = -1;

	while (str[++i] != '\0') {
		cyBot_sendByte(str[i]);
	}
}
