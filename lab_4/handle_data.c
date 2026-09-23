#include "handle_data.h"

#define MAX_SECTIONS 10


void clean_data (struct SensorData sensor_data[], int len) {
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
 * Converts sensor-data file to an array of SensorData structs
 * @param filename
 * @param return_arr pointer to SensorData Array
 * @returns length of SensorData Array
 */
int file_to_SD(char* filename, struct SensorData* return_arr) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Could not open file\n");
        return 1;
    }

    char line[35];
    int line_count = 0;
    char* deg_str, line_str;

    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s\n", line);
        line_count++;
    }

    fclose(file);

    return line_count;
}

void aprox_sizes(struct SensorData sensor_data[], int len) {
    // Find edges

    int i;

    struct SensorDataSection *sections = malloc(sizeof(struct SensorDataSection) * MAX_SECTIONS);
    int section_count = 1;
    int start = 0;

    for (i = 1; i < len; i++) {
        if (sensor_data[i].dist != sensor_data[i-1].dist) {
            printf("Edge found at %02d\n", sensor_data[i].ang);
            sections[section_count - 1].start_ang = sensor_data[start].ang;
            sections[section_count - 1].end_ang = sensor_data[i].ang;

            sections[section_count - 1].dist = sensor_data[start].dist;

            section_count++;
            start = i;

        }
    }

    sections[section_count - 1].start_ang = sensor_data[start].ang;
    sections[section_count - 1].end_ang = sensor_data[len - 1].ang;
    sections[section_count - 1].dist = sensor_data[start].dist;

    printf("Found %02d sections\n", section_count);

    int object_num = 1;

    int mid, width; 
    struct SensorDataSection section;

    printf("Object#\t\tAngle\t\tDistance\tWidth\r\n");
    for (i = 0; i < section_count; i++) {
        section = sections[i];
        // printf("Found section from %3d to %3d of %6.2f\n", section.start_ang, section.end_ang, section.dist);

        if (sections[i].dist < 135) {
            width = section.end_ang - section.start_ang;
            mid = width / 2 + section.start_ang;

            printf("%d\t\t%d\t\t%d\t\t%d\r\n", object_num++, mid, (int)floor(section.dist), width);
        }
    }
}

/*
#include "sensor-data.h"
int main(int argc, char const *argv[]) {
    struct SensorData* sensor_data_ptr = malloc(sizeof(struct SensorData) * 91);
    
    int i;
    for (i = 0; i < 91; i++) {
        struct SensorData temp;
        
        temp.ang = i * 2;
        temp.dist = sensor_data_array[i];
        
        sensor_data_ptr[i] = temp;
    }
    
    clean_data(sensor_data_ptr, 91);
    
    // Chop off data
    sensor_data_ptr = &(sensor_data_ptr[2]);
    
    for (i = 0; i < 89; i++){
        printf("%02d: %6.2f vs og of %6.2f\r\n", sensor_data_ptr[i].ang, sensor_data_ptr[i].dist, sensor_data_array[i+2]);
    }
    
    aprox_sizes(sensor_data_ptr, 89);
    
    return 0;
}
*/
