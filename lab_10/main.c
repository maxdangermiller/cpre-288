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
        timer_waitMillis(2000);

        lcd_printf("Sending Ping!\n");
        // ping_read();

        ping_val = ping_read();

        lcd_printf("%06d\n", ping_val);
	}

	return 0;
}
 
