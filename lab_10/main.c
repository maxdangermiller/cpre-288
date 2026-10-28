/**
 * CprE 288 - Lab 10
 * @authors Max Miller, Miles Davies
 */

#include "ping.h"
#include "Timer.h"
#include "lcd.h"

int main() {
	timer_init();
	ping_init();

	while (1) {
        timer_waitMillis(2000);

        ping_read();
	}

	return 0;
}
 
