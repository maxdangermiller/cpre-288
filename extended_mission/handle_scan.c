/**
 * @name Handle Scan
 * @headerfile handle_scan.h
 * @author Max Miller
 */
#include "handle_scan.h"
#include "uart.h"
#include "adc.h"
#include "ping.h"
#include "servo.h"


/**
 * Do scan on CyBot and save the data
 * @param sensor_data
 */
void do_scan(SensorData_t sensor_data[]) {
	int i;

	float sound_distance, ir_distance;

	float degrees = 0.0;

	for (i = 0; i < SCAN_STEPS; i++) {
		degrees = i * SCAN_RESOLUTION + MIN_ANGLE;

		servo_move(degrees);

		ir_distance = adc_get_dist(adc_read());
		sound_distance = ping_cycles_to_cm(ping_read());
	
		sensor_data[i].s_dist = (float)(sound_distance - CYBOT_FRONT_TO_SENSOR);
		sensor_data[i].ir_dist = (float)(ir_distance - CYBOT_FRONT_TO_SENSOR);
	
        sensor_data[i].ang = degrees;
	}
}


/**
 * Do scan on CyBot for only the angle directly ahead and return the sound data
 * @param sensor_data
 */
void scan_ahead(double *dist_ahead, int use_IR) {

	servo_move((MAX_ANGLE - MIN_ANGLE) / 2);

	float sound_d = ping_cycles_to_cm(ping_read());
	
	// subtract distance from sensor to front of CyBot
	*dist_ahead = (float)(sound_d - CYBOT_FRONT_TO_SENSOR);

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
