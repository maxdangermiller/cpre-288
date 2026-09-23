#ifndef PART_5_H

#define PART_5_H

#include "cyBot_Scan.h"
#include "cyBot_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "handle_data.h"

void part_5();

int find_objects(struct SensorData sensor_data[], int len, struct SensorDataSection* sections);
void do_scan_2(struct SensorData sensor_data[]);
void load_scan(struct SensorData sensor_data[]);
void send_scan_2(struct SensorData sensor_data[]);
void cyBot_sendString_2(char* str);
void display_objects(struct SensorDataSection* sections, int section_count);
int get_smallest_index(struct SensorDataSection* sections, int section_count);
float get_section_len(struct SensorDataSection section);

#endif
