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
#define MAX_SECTIONS 10
#define MAX_OBJECTS 10


typedef struct SensorData {
	int ang;
	float dist;
} SensorData_t;

typedef struct SensorDataSection {
	int start_ang;
	int end_ang;
	float dist;
} SensorDataSection_t;

typedef struct ObjectInfo {
	int num;    // Number
	float mp;   // Midpoint
	float dist; // Distance
	float len;  // Length
} ObjectInfo_t;

void clean_data (SensorData_t sensor_data[], int len);
int find_objects(SensorData_t sensor_data[], int len, ObjectInfo_t *objects);
int identify_object(ObjectInfo_t *object, SensorDataSection_t *data_section);  // Helper function for find_objects
float get_linear_size(SensorDataSection_t *data_section);

int get_smallest_obj(ObjectInfo_t *objects, int len);

void send_object_table(ObjectInfo_t *objects, int object_count);
void cyBot_sendString(char* str);

#endif
