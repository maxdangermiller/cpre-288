#ifndef CYBOT_SCAN_CAL_H
#define CYBOT_SCAN_CAL_H

#include "cyBot_Scan.h"
#include "uart_utils.h"
#include "timer.h"
#include <math.h>

// #define USE_CYBOT_01
#define USE_CYBOT_11
// #define USE_CYBOT_17
// #define USE_CYBOT_22
// #define USE_CYBOT_24
// #define USE_CYBOT_25
// #define USE_CYBOT_26

void cyBot_IR_Calibrate();
double cyBot_est_IR_dist(double ir_dist);
void cyBot_init_Scan_cust(int feature);

#endif
