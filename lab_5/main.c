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



int main(void) {
	oi_t *cyBot = oi_alloc();

    oi_init(cyBot);
    oi_setMotorCalibration(1.0, 1.0);

	// rot_calibrate(cyBot);

	cyBot_uart_init();

	cyBOT_init_Scan(0b111);
	// Values for CyBot #1
	// right_calibration_value = 232750;
	// left_calibration_value = 1225000;
	
	// CyBot #24
    // right_calibration_value = 253750;
    // left_calibration_value = 1235500;

	// Values for CyBot #26
	right_calibration_value = 311500;
	left_calibration_value = 1309000;

	oi_setWheels(0, 0);


	struct SensorData* sensor_data_ptr = (struct SensorData*)malloc(sizeof(struct SensorData) * SCAN_STEPS);
    struct ObjectInfo *objects = (struct ObjectInfo*)malloc(sizeof(struct ObjectInfo) * MAX_OBJECTS);
	
	// struct ObjectInfo object;
	// int i;
	char cmd = 0;
	char str[50];

	while (true) {
		cmd = (char)cyBot_getByte();

		
		if (cmd == 'm') {				// M key to map
			turn_abs(cyBot, 0);
			do_scan(sensor_data_ptr);
			// load_scan(sensor_data_ptr, SCAN_STEPS);

			cyBot_sendString("**Raw noisy sensor data**\r\n");
			send_scan(sensor_data_ptr);

			clean_data(sensor_data_ptr, SCAN_STEPS);

			cyBot_sendString("** Cleaned data **\r\n");
			send_scan(sensor_data_ptr);

			int object_count = find_objects(&(sensor_data_ptr[2]), SCAN_STEPS - 2, objects);

			send_object_table(objects, object_count);

			int smallest_index = get_smallest_obj(objects, object_count);
			sprintf(str, "Smallest Object is #%d\r\n", objects[smallest_index].num);
			cyBot_sendString(str);

			turn_abs(cyBot, objects[smallest_index].mp - 90);
		}
		
		else if (cmd == 'w') {			// W key to go forward
            move_forward(cyBot, 500);
        }

        else if (cmd == 'a') {			// A key to go left
            turn_ccw(cyBot, 90);
        }

        else if (cmd == 's') {			// S key to go backward
            move_backward(cyBot, 500);
        }

        else if (cmd == 'd') {			// D key to go right
            turn_cw(cyBot, 90);
        }

		else if (cmd == 'q') {			// Q key to get current position
			struct Pos pos = getPosition();

			sprintf(str, "Current Position (mm) of the CyBot is: ", pos.x, pos.y, pos.a);
            cyBot_sendString(str);

			sprintf(str, "(%.2lf, %.2lf) and is rotated at %.1lf\r\n", pos.x, pos.y, pos.a);
            cyBot_sendString(str);
        }

		else if (cmd == 'r') {
		    move_abs(cyBot, 0.0, 0.0, true);
		    turn_abs(cyBot, 0.0);
		}

		else if (cmd == 'e') {			// E key to EXIT
			break;
		}
	}

	free(sensor_data_ptr);
	free(objects);
	
}
