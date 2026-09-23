#include "part3.h"

#include "cyBot_Scan.h"
#include "cyBot_uart.h"
#include "Timer.h"
#include "lcd.h"
#include <stdio.h>
#include <stdlib.h>


int part_3() {
	timer_init();
	lcd_init();
	cyBot_uart_init();
	cyBOT_init_Scan(0b111);

	// COMMENT OUT WHEN DONE CALIBRATING USING LCD
	// cyBOT_SERVO_cal();

	// End the program here if we're still calibrating
	// return 0;

	// Values for CyBot #1
	right_calibration_value = 232750;
	left_calibration_value = 1225000;
	
	// Values for CyBot #26
	right_calibration_value = 311500;
	left_calibration_value = 1309000;

	int i;
	cyBOT_Scan_t scan;

	for(i = 0; i < 180; i += 2) {
		cyBOT_Scan(i, &scan);
	}
	
	
	return 0;
}
