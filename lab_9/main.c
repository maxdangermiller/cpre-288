#include "adc.h"

#define PART_1				// ADC Raw Test (Make sure to disable EN_HARDWARE_SAMPLING in adc.h)
// #define PART_2_A        	// IR Calibration
// #define PART_2_B			// IR Distances


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


int main() {
	timer_init();
	lcd_init();
	adc_init();

	int raw;
	float dist;

	while (true) {
		raw = adc_read();
		dist = adc_get_dist(raw);

		lcd_printf("RAW: %4d\nDIST:%6.2f\n", raw, dist);
		
		timer_waitMillis(100);
	}
	
	return 0;
}

#endif
