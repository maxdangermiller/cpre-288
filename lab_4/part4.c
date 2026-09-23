#include "part4.h"
#include "part2.h"


#define SCAN_RESOLUTION 2
#define MIN_ANGLE 0
#define MAX_ANGLE 180
#define SCAN_STEPS (MAX_ANGLE - MIN_ANGLE) / SCAN_RESOLUTION + 1



int part_4() {
	cyBot_uart_init();
	cyBOT_init_Scan(0b111);

	char c;
	float* scan_data = (float*)malloc(sizeof(float) * SCAN_STEPS);

	// Values for CyBot #1
	// right_calibration_value = 232750;
	// left_calibration_value = 1225000;

	// Values for CyBot #26
    right_calibration_value = 311500;
    left_calibration_value = 1309000;

	while (1) {
		c = (char)cyBot_getByte();

		if (c == 'm') {
			do_scan(scan_data);

			
		}

		if (c == 'e') {
            break;


        }

		send_scan(scan_data);
	}
	

	return 0;
}
