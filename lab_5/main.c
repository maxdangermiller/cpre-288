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
static oi_t *cyBot;
static char str[50];

static double smallest_ang = 90;

void map(int point_en);
void get_pos();
void return_to_home();
void send_help();



int main(void) {
	timer_init();
	cyBot_uart_init();

	cyBot = oi_alloc();
    oi_init(cyBot);


	sprintf(str, "\r\n\r\nCprE288 Lab 5 - Created by Max Miller\r\n\r\n\r\n");
	cyBot_sendString(str);


	cyBOT_init_Scan(0b111);


    // oi_setMotorCalibration(1.0, 1.0);
	// rot_calibrate(cyBot);
	
	// Stuff for servo calibration
	// lcd_init();
	// cyBOT_SERVO_cal();


	// Values for CyBot #1
	// right_calibration_value = 232750;
	// left_calibration_value = 1225000;
	
	// CyBot #17
    right_calibration_value = 248500;
    left_calibration_value = 1204000;

	// CyBot #22
    // right_calibration_value = 248500;
    // left_calibration_value = 1267000;

	// CyBot #24
    // right_calibration_value = 253750;
    // left_calibration_value = 1235500;

	// Values for CyBot #25
    // right_calibration_value = 285250;
    // left_calibration_value = 1246000;

	// Values for CyBot #26
	// right_calibration_value = 311500;
	// left_calibration_value = 1309000;


    // Stop it if it was moving!
	oi_setWheels(0, 0);
	
	char cmd = 0;
	int running = true;
	
	/*
	UART COMMANDS:

	M: MAP
	P: MAP & POINT
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
				map(false);
				break;

            // Map & Point
            case 'p':
                turn_rel(cyBot, smallest_ang - 90.0);
                break;

			
			// Forward
			case 'w':
				move_forward(cyBot, 20);
				break;
			
			// Left
			case 'a':
				turn_ccw(cyBot, 45);
				break;

			// running
			case 's':
				move_backward(cyBot, 20);
				break;
			
			// Right
			case 'd':
				turn_cw(cyBot, 45);
				break;
			
			// Send current position
			case 'q':
				get_pos(cyBot);
				break;

			// Return to start position
			case 'r':
				return_to_home(cyBot);
				break;

			case 'h':
			    send_help();
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
void map(int point_en) {
	// turn_abs(cyBot, 0.0);
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

	smallest_ang =  (double)objects[smallest_index].mp;

	if (point_en) {
	    turn_rel(cyBot, (double)objects[smallest_index].mp - 90.0);
	}
}


/**
 * @name Get Pos
 * @brief Get position and send it over UART
 */
void get_pos() {
	// TODO: Unimplemented
}


/**
 * @name Return to Home
 * @brief Go back to the origin
 */
void return_to_home() {
	// TODO: Unimplemented
}


void send_help() {
    /*
    UART COMMANDS:

    M: MAP
    P: MAP & POINT
    W: Forward
    A: Left
    S: Backward
    D: Right
    Q: GET CURRENT POSITION
    R: RETURN TO START POINT
    E: EXIT
    */
    cyBot_sendString("\r\nUART COMMANDS: \r\n\r\n");
    cyBot_sendString("M: MAP\r\n");
    cyBot_sendString("P: MAP & POINT\r\n");
    cyBot_sendString("W: Forward\r\n");
    cyBot_sendString("A: Left\r\n");
    cyBot_sendString("S: Backward\r\n");
    cyBot_sendString("D: Right\r\n");
    cyBot_sendString("Q: GET CURRENT POSITION\r\n");
    cyBot_sendString("R: RETURN TO START POINT\r\n");
    cyBot_sendString("E: EXIT\r\n");
}
