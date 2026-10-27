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

	double degrees = 0.0;

	for (i = 0; i < SCAN_STEPS; i++) {
		degrees = i * SCAN_RESOLUTION + MIN_ANGLE;

		cyBOT_Scan(degrees, &data);
	
		sensor_data[i].s_dist = (float)(data.sound_dist - CYBOT_FRONT_TO_SENSOR);
		sensor_data[i].ir_dist = (float)(adc_get_dist(adc_read()) - CYBOT_FRONT_TO_SENSOR);
	
        sensor_data[i].ang = degrees;
	}
}


/**
 * Do scan on CyBot for only the angle directly ahead and return the sound data
 * @param sensor_data
 */
void scan_ahead(double *dist_ahead, int use_IR) {
	cyBOT_Scan_t data;

	cyBOT_Scan(90 + MIN_ANGLE, &data); 
	
	// subtract distance from sensor to front of CyBot
	*dist_ahead = (float)(data.sound_dist - 6.0);

}

/**
 * Send scan back to the uart connection
 * @param sensor_data
 */
void send_scan(SensorData_t sensor_data[]) {
	
	int i;
	char str[50];

	sprintf(str, "Angle(Degrees)\tSound Distance(cm)\tIR Distance (cm)\r\n");

	uart_sendStr(str);

	for(i = 0; i < SCAN_STEPS; i++) {
		// Print Data Line
		sprintf(str, "%d\t\t%f\t\t%f\r\n", sensor_data[i].ang, sensor_data[i].s_dist, sensor_data[i].ir_dist);

		uart_sendStr(str);
	}
}


void servo_point(double angle) {
	cyBOT_Scan_t data;

	cyBOT_Scan((int)angle, &data); 
}
