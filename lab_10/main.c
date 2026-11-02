/**
 * CprE 288 - Lab 10
 * @authors Max Miller, Miles Davies
 */

#include "ping.h"
#include "Timer.h"
#include "lcd.h"

int main() {
	timer_init();
	lcd_init();
	ping_init();

	int cycles;

	while (1) {
        timer_waitMillis(300);					// Take a reading every 300 ms

        cycles = ping_read();

        lcd_printf("Cycles: %d\nTime: %.3f ms\nDist: %.1f cm\nOverflows: %d",
                   cycles, ping_cycles_to_ms(cycles), ping_cycles_to_cm(cycles), ping_get_overflows());
	}

	return 0;
}
 
