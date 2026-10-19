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

	double degrees = 0.0;

	for (i = 0; i < SCAN_STEPS; i++) {

		sound_mes = 0.0;
		ir_mes = 0.0;
		degrees = i * SCAN_RESOLUTION + MIN_ANGLE;

		// Scan 3 times
		for (j = 0; j < SCANS_PER_ANG; j++) {
			cyBOT_Scan(degrees, &data);
	
			// subtract distance from sensor to front of CyBot
			sound_mes += (double)(data.sound_dist - CYBOT_FRONT_TO_SENSOR);
			ir_mes += (double)(cyBot_est_IR_dist(data.IR_raw_val) - CYBOT_FRONT_TO_SENSOR);
		}

		
		sensor_data[i].s_dist = (float)(sound_mes / (double)SCANS_PER_ANG);
		sensor_data[i].ir_dist = (float)(ir_mes / (double)SCANS_PER_ANG);
	
        sensor_data[i].ang = degrees;
	}
}


/**
 * Do scan on CyBot for only the angle directly ahead and return the sound data
 * @param sensor_data
 */
void scan_ahead(double *dist_ahead, int use_IR) {
	int i;
	//char str[50];
	cyBOT_Scan_t data;
	double mes = 0.0;

	cyBOT_Scan(90 + MIN_ANGLE, &data); 
	timer_waitMillis(50); //initial turn then delay is necessary to make sure sensor gets accurate info

	// Scan 3 times
	for (i = 0; i < SCANS_PER_ANG; i++) {
		cyBOT_Scan(90 + MIN_ANGLE, &data);
		if(use_IR){
			mes += (double)(cyBot_est_IR_dist(data.IR_raw_val) - 6.0);
		}
		else {
			mes += (double)(data.sound_dist - 6.0);
		}
	}
	
	// subtract distance from sensor to front of CyBot
	*dist_ahead = (float)(mes / (double)SCANS_PER_ANG);

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
