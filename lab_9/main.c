#include "adc.h"

// #define PART_1			// ADC Raw Test (Make sure to disable EN_HARDWARE_SAMPLING in adc.h)
// #define PART_2_A        	// IR Calibration
#define PART_2_B			// IR Distances


#ifdef PART_1

#include "Timer.h"
#include "lcd.h"

void main() {
	timer_init();
	lcd_init();

	lcd_printf("ADC INIT\n");

	adc_init();

	while(true){
		lcd_printf("R0: %d\n", adc_read());
		timer_waitMillis(100);
	}
}

#elif defined(PART_2_A) 

int main() {
	adc_calibrate();
	
	return 0;
}

#elif defined(PART_2_B) 

#include "Timer.h"
#include "lcd.h"
#include "cyBot_Scan.h"
#include "servo.h"


int main() {
	timer_init();
	lcd_init();
	adc_init();
	cyBOT_init_Scan(0b011);

	int raw;
	float dist;

	cyBOT_Scan_t data;

	left_calibration_value = SERVO_LEFT_CAL;
	right_calibration_value = SERVO_RIGHT_CAL;

	while (true) {
		raw = adc_read();
		dist = adc_get_dist(raw);

		cyBOT_Scan(90, &data);

		lcd_printf("RAW: %4d\nDIST:%6.2f/%6.2fcm\n", raw, dist, data.sound_dist);
		
		timer_waitMillis(100);
	}
	
	return 0;
}

#endif
