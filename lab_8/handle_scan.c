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
	int i, j;

	cyBOT_Scan_t data;

	double sound_mes = 0.0;
	double ir_mes = 0.0;

	for (i = 0; i < SCAN_STEPS; i++) {

		sound_mes = 0.0;
		ir_mes = 0.0;

		// Scan 3 times
		for (j = 0; j < SCANS_PER_ANG; j++) {
			cyBOT_Scan(i * SCAN_RESOLUTION + MIN_ANGLE, &data);
	
			sound_mes += (double)(data.sound_dist - 6.0);
			ir_mes += (double)(cyBot_est_IR_dist(data.IR_raw_val) - 6.0);
		}
		
        sensor_data[i].ang = i * SCAN_RESOLUTION + MIN_ANGLE;
        // subtract distance from sensor to front of CyBot
		sensor_data[i].s_dist = (float)(sound_mes / (double)SCANS_PER_ANG);
		sensor_data[i].ir_dist = (float)(ir_mes / (double)SCANS_PER_ANG);
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
