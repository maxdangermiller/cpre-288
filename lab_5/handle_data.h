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


#define JUMP_THRESHOLD 50
#define MAX_SECTIONS 10
#define MAX_OBJECTS 10


struct SensorData {
	int ang;
	float dist;
};

struct SensorDataSection {
	int start_ang;
	int end_ang;
	float dist;
};

struct ObjectInfo {
	int num;    // Number
	float mp;   // Midpoint
	float dist; // Distance
	float len;  // Length
};

void clean_data (struct SensorData sensor_data[], int len);
int find_objects(struct SensorData sensor_data[], int len, struct ObjectInfo* objects);
int identify_object(struct ObjectInfo *object, struct SensorDataSection *data_section);  // Helper function for find_objects
float get_linear_size(struct SensorDataSection *data_section);

int get_smallest_obj(struct ObjectInfo *objects, int len);

void send_object_table(struct ObjectInfo *objects, int object_count);
void cyBot_sendString(char* str);

#endif
