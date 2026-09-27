/**
 * @name Handle Scan
 * @file handle_scan.c
 * @author Max Miller
 */
#ifndef HANDLE_SCAN_H

#define HANDLE_SCAN_H

#include "handle_data.h"
#include "sensor-data.h"
#include "cyBot_Scan.h"
#include "cyBot_uart.h"

#define SCAN_RESOLUTION 2
#define MIN_ANGLE 0
#define MAX_ANGLE 180
#define SCAN_STEPS (MAX_ANGLE - MIN_ANGLE) / SCAN_RESOLUTION + 1

void load_scan(SensorData_t sensor_data[], int len);
void do_scan(SensorData_t sensor_data[]);
void send_scan(SensorData_t sensor_data[]);

#endif
