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

	int ping_val;

	while (1) {
        timer_waitMillis(300);

        ping_val = ping_read();
		
        lcd_printf("Cycles: %d\nTime: %.3f ms\nDist: %.1f cm\nOverflows: %d", ping_val, ping_cycles_to_ms(ping_val), ping_cycles_to_cm(ping_val), ping_get_overflows());
	}

	return 0;
}
 
