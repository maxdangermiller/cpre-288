/**
 * @name Main File
 * @brief Main file for CprE 288 Lab 5
 * @author Max Miller
 */

#include "handle_data.h"
#include "sensor-data.h"
#include "handle_scan.h"
#include "cyBot_Scan.h"
#include "cyBot_uart.h"
#include "movement.h"
#include "timer.h"

static SensorData_t sensor_data_ptr[SCAN_STEPS];
static ObjectInfo_t objects[MAX_OBJECTS];
static char str[50];

Pos_t target_pos;

void map(oi_t *cyBot);
void get_pos(oi_t *cyBot);
void return_to_home(oi_t *cyBot);



int main(void) {
	oi_t *cyBot = oi_alloc();
    oi_init(cyBot);

	cyBot_uart_init();

	sprintf(str, "\r\n\r\nCprE288 Lab 5 - Created by Max Miller\r\n\r\n\r\n");
	cyBot_sendString(str);


	cyBOT_init_Scan(0b011);


    // oi_setMotorCalibration(1.0, 1.0);
	// rot_calibrate(cyBot);
	
	

	// Values for CyBot #1
	// right_calibration_value = 232750;
	// left_calibration_value = 1225000;
	
	// CyBot #24
    // right_calibration_value = 253750;
    // left_calibration_value = 1235500;

	// Values for CyBot #26
	right_calibration_value = 311500;
	left_calibration_value = 1309000;


    // Stop it if it was moving!
	oi_setWheels(0, 0);
	
	char cmd = 0;
	int running = true;

	target_pos.x = 0;
	target_pos.y = 0;
	
	/*
	UART COMMANDS:

	M: MAP
	W: Forward
	A: Left
	S: Backward
	D: Right
	Q: GET CURRENT POSITION
	R: RETURN TO START POINT
	E: EXIT
	*/

	while (running) {
		cmd = (char)cyBot_getByte();

		sprintf(str, "Pressed: %c\r\n\r\n", cmd);
		cyBot_sendString(str);


		switch (cmd) {
			// Map
			case 'm':
				map(cyBot);
				break;
			
			// Forward
			case 'w':
				move_forward(cyBot, 500);
				break;
			
			// Left
			case 'a':
				turn_ccw(cyBot, 90);
				break;

			// running
			case 's':
				move_backward(cyBot, 500);
				break;
			
			// Right
			case 'd':
				turn_cw(cyBot, 90);
				break;
			
			// Send current position
			case 'q':
				get_pos(cyBot);
				break;

			// Return to start position
			case 'r':
				return_to_home(cyBot);
				break;

			// Exit
			case 'e':
				running = false;
				break;

			default:
			    break;
		}
	}

	free(sensor_data_ptr);
	free(objects);
	free(cyBot);
	
}


/**
 * @name Map
 * @brief Maps objects and points to the smallest one
 */
void map(oi_t *cyBot) {
	turn_abs(cyBot, 0);
	do_scan(sensor_data_ptr);
	// load_scan(sensor_data_ptr, SCAN_STEPS);

	// Send Raw Sensor Data
	cyBot_sendString("**Raw noisy sensor data**\r\n");
	send_scan(sensor_data_ptr);

	clean_data(sensor_data_ptr, SCAN_STEPS);

	// Send Cleaned Sensor Data
	cyBot_sendString("** Cleaned data **\r\n");
	send_scan(sensor_data_ptr);
	
	// Determine Object count
	int object_count = find_objects(&(sensor_data_ptr[2]), SCAN_STEPS - 2, objects);

	// Send the object table to PuTTy
	send_object_table(objects, object_count);

	int smallest_index = get_smallest_obj(objects, object_count);
	sprintf(str, "Smallest Object is #%d\r\n", objects[smallest_index].num);
	cyBot_sendString(str);

	turn_abs(cyBot, objects[smallest_index].mp - 90.0);
}


/**
 * @name Get Pos
 * @brief Get position and send it over UART
 */
void get_pos(oi_t *cyBot) {
	Pos_t pos = getPosition();

	sprintf(str, "(%.2lf, %.2lf) and is rotated at %.1lf\r\n", pos.x, pos.y, pos.a);
	cyBot_sendString(str);
}


/**
 * @name Return to Home
 * @brief Go back to the origin
 */
void return_to_home(oi_t *cyBot) {
	driveToPoint(cyBot, 0.0, 0.0);
	// Pos_t pos = getPosition();
	// move_abs(cyBot, 0.0, 0.0, true);
	// turn_abs(cyBot, pos.a);
}
