#include "part5.h"

#include "cyBot_Scan.h"
#include "cyBot_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "handle_data.h"
#include "sensor-data.h"


#define SCAN_RESOLUTION 2
#define MIN_ANGLE 0
#define MAX_ANGLE 180
#define SCAN_STEPS (MAX_ANGLE - MIN_ANGLE) / SCAN_RESOLUTION + 1

#define MAX_SECTIONS 10

void part_5() {
    cyBot_uart_init();
    
    struct SensorData* sensor_data_ptr = malloc(sizeof(struct SensorData) * SCAN_STEPS);
    struct SensorDataSection *sections = malloc(sizeof(struct SensorDataSection) * MAX_SECTIONS);

    int obj_count;
    int smallest_index, ang_width, mid;

    cyBOT_Scan_t data;


    cyBOT_init_Scan(0b111);
    right_calibration_value = 311500;
    left_calibration_value = 1309000;
    

    char c;
    cyBOT_Scan(0, &data);

    while (1) {
        c = (char)cyBot_getByte();

        if (c == 'm') {
            // do_scan_2(sensor_data_ptr);
            load_scan(sensor_data_ptr);

            clean_data(sensor_data_ptr, SCAN_STEPS);

            send_scan_2(sensor_data_ptr);

            // Chop off first two SensorData, they are always weird
            obj_count = find_objects(&(sensor_data_ptr[2]), SCAN_STEPS - 2, sections);

            display_objects(sections, obj_count);

            smallest_index = get_smallest_index(sections, obj_count);
            ang_width = sections[smallest_index].end_ang - sections[smallest_index].start_ang;
            mid = ang_width / 2 + sections[smallest_index].start_ang;
            cyBOT_Scan(mid, &data);

        }

        if (c == 'e') {
            break;


        }
    }

    free(sensor_data_ptr);
    free(sections);

}

/**
 * Approximate Sizes of objects and send them to the CyBot
 */
int find_objects(struct SensorData sensor_data[], int len, struct SensorDataSection* sections) {
    // Find edges

    int i;

    int section_count = 0;
    int start = 0;


    for (i = 1; i < len; i++) {

        // Edge found at sensor_data[i]
        if (sensor_data[i].dist != sensor_data[i-1].dist) {
            // printf("Edge found at %02d\n", sensor_data[i].ang);

            if (sensor_data[start].dist < 135) {
                sections[section_count].start_ang = sensor_data[start].ang;
                sections[section_count].end_ang = sensor_data[i].ang;

                sections[section_count].dist = sensor_data[start].dist;
                
                section_count++;

            }

            start = i;

            if (section_count >= MAX_SECTIONS - 1) {
                break;
            }

        }
    }

    sections[section_count].start_ang = sensor_data[start].ang;
    sections[section_count].end_ang = sensor_data[len - 1].ang;
    sections[section_count].dist = sensor_data[start].dist;
    section_count++;

    return section_count;
}


/**
 * Display Objects
 * @param sections
 * @param section_count sections length
 */
void display_objects(struct SensorDataSection* sections, int section_count) {
    int object_num = 1;

    int mid, ang_width, i; 
    struct SensorDataSection section;
    char str[50];

    sprintf(str, "\r\nObject#\t\tAngle\t\tDistance\tWidth\r\n");
    cyBot_sendString_2(str);

    int smallest_index = get_smallest_index(sections, section_count);
    int smallest_num = -1;

    for (i = 0; i < section_count; i++) {
        section = sections[i];
        // printf("Found section from %3d to %3d of %6.2f\n", section.start_ang, section.end_ang, section.dist);

        if (i == smallest_index) {
            smallest_num = object_num;
        }

        if (section.dist < 135) {
            ang_width = section.end_ang - section.start_ang;
            mid = ang_width / 2 + section.start_ang;

            sprintf(str, "%d\t\t%d\t\t%.2f\t\t%.2f\r\n", object_num++, mid, section.dist, get_section_len(section));

            cyBot_sendString_2(str);

        }

    }

    sprintf(str, "Smallest Object is #%d\r\n", smallest_num);
    cyBot_sendString_2(str);

    /*
    section = sections[smallest_index];

    ang_width = section.end_ang - section.start_ang;
    mid = ang_width / 2 + section.start_ang;

    sprintf(str, "%d\t\t%d\t\t%.2f\t\t%.2f\r\n", smallest_num, mid, section.dist, get_section_len(section));
    cyBot_sendString_2(str);
    */
}


int get_smallest_index(struct SensorDataSection* sections, int section_count) {
    
    int i;
    
    int min_index = 0;
    float min_size = get_section_len(sections[0]);

    for (i = 1; i < section_count; i++) {
        if (get_section_len(sections[i]) < min_size && sections[i].dist < 135) {
            min_size = get_section_len(sections[i]);
            min_index = i;
        }
    }

    return min_index;
} 

float get_section_len(struct SensorDataSection section) {
    float a = section.dist;
    float b = section.dist;

    return sqrtf(a*a + b*b - 2 * a * b * cosf(section.end_ang - section.start_ang));
}


/**
 * Do scan on CyBot and save the data
 * @param sensor_data
 */
void do_scan_2(struct SensorData sensor_data[]) {
	int i;

	cyBOT_Scan_t data;

	for (i = 0; i < SCAN_STEPS; i++) {
		cyBOT_Scan(i * SCAN_RESOLUTION + MIN_ANGLE, &data);
		
        sensor_data[i].ang = i * SCAN_RESOLUTION + MIN_ANGLE;
		sensor_data[i].dist = data.sound_dist;
	}
}

void load_scan(struct SensorData sensor_data[]) {
    int i;
    for (i = 0; i < SCAN_STEPS; i++) {
        sensor_data[i].ang = i * 2;
        sensor_data[i].dist = sensor_data_array[i];
    }
}

/**
 * Send scan back to the uart connection
 * @param sensor_data
 */
void send_scan_2(struct SensorData sensor_data[]) {
	
	int i;
	char str[35];

	sprintf(str, "Angle(Degrees)\tDistance(mm)\r\n");

	cyBot_sendString_2(str);

	for(i = 0; i < SCAN_STEPS; i++) {
		// Print Data Line
		sprintf(str, "%d\t\t\t%lf\r\n", sensor_data[i].ang, sensor_data[i].dist);

		cyBot_sendString_2(str);
	}
}

/**
 * cyBot send string back to user
 * @param str
 */
void cyBot_sendString_2(char* str) {
	int i = -1;

	while (str[++i] != '\0') {
		cyBot_sendByte(str[i]);
	}
}
