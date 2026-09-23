#ifndef HANDLE_DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define HANDLE_DATA_H

#define JUMP_THRESHOLD 10


struct SensorData {
    int ang;
    float dist;
};

struct SensorDataSection {
    int start_ang;
    int end_ang;
    float dist;
};

void clean_data (struct SensorData sensor_data[], int len);

#endif
