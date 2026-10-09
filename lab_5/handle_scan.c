/**
 * @name Handle Scan
 * @headerfile handle_scan.h
 * @author Max Miller
 */
#include "handle_scan.h"

/**
 * @param sensor_data
 * @param len
 */
void load_scan(SensorData_t sensor_data[], int len) {
    int i;
    
    for (i = 0; i < len; i++) {
        sensor_data[i].ang = i * 2;
        sensor_data[i].s_dist = sensor_data_array[i];
    }
}

/**
 * Do scan on CyBot and save the data
 * @param sensor_data
 */
void do_scan(SensorData_t sensor_data[]) {
	int i;

	cyBOT_Scan_t data;

	for (i = 0; i < SCAN_STEPS; i++) {
		cyBOT_Scan(i * SCAN_RESOLUTION + MIN_ANGLE, &data);
		
        sensor_data[i].ang = i * SCAN_RESOLUTION + MIN_ANGLE;
        // subtract distance from sensor to front of CyBot
		sensor_data[i].s_dist = data.sound_dist - 6.0;
		sensor_data[i].ir_dist = (float)(cyBot_est_IR_dist(data.IR_raw_val) - 6.0);
	}
}

/**
 * Send scan back to the uart connection
 * @param sensor_data
 */
void send_scan(SensorData_t sensor_data[]) {
	
	int i;
	char str[50];

	sprintf(str, "Angle(Degrees)\tSound Distance(cm)\tIR Distance (cm)\r\n");

	cyBot_sendString(str);

	for(i = 0; i < SCAN_STEPS; i++) {
		// Print Data Line
		sprintf(str, "%d\t\t%f\t\t%f\r\n", sensor_data[i].ang, sensor_data[i].s_dist, sensor_data[i].ir_dist);

		cyBot_sendString(str);
	}
}
