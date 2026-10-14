/**
 * @name Handle Data
 * @file handle_data.c
 * @author Max Miller
 */
#ifndef HANDLE_DATA_H
#define HANDLE_DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define JUMP_THRESHOLD 25
#define OUTLIER_THRESHOLD 20
#define MAX_SECTIONS 10
#define MAX_OBJECTS 10


typedef struct SensorData {
	int ang;		// Angle (degrees)
	float s_dist;	// Sound Distance (cm)
	float ir_dist;	// IR Distance (cm)
} SensorData_t;

typedef struct SensorDataSection {
	int start_ang;	// Start Angle (degrees)
	int end_ang;	// End Angle (degrees)
	float s_dist;	// Sound Distance (cm)
	float ir_dist;	// IR Distance (cm)
} SensorDataSection_t;

typedef struct ObjectInfo {
	int num;    	// Number (index)
	float mp;   	// Midpoint (degrees)
	float s_dist; 	// Sound Distance (cm)
	float ir_dist;	// IR Distance (cm)
	float len;  	// Length 
} ObjectInfo_t;

void clean_data (SensorData_t sensor_data[], int len);
int find_objects(SensorData_t sensor_data[], int len, ObjectInfo_t *objects);
int identify_object(ObjectInfo_t *object, SensorDataSection_t *data_section);  // Helper function for find_objects
float get_linear_size(SensorDataSection_t *data_section);

int get_smallest_obj(ObjectInfo_t *objects, int len);

void send_object_table(ObjectInfo_t *objects, int object_count);
void cyBot_sendString(char* str);

#endif
