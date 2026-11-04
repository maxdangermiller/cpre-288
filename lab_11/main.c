/**
 * @brief Main File for Lab 11
 * @author Max Miller, Miles Davies
 */
#include "servo.h"
#include "Timer.h"
#include "lcd.h"
#include "button.h"

extern volatile int button_event;
extern volatile int button_num;

static char dir_str[4];



int main(void) {
	timer_init();
	lcd_init();
	servo_init();
	button_init();

	int dir = 1;
	float angle = 90.0;


	int pulse = servo_move(angle);

	while (1) {
		if (dir > 0) 	{ strcpy(dir_str, "CCW");	}
		else 			{ strcpy(dir_str, "CW");	}

        lcd_printf("Direction: %s\nMatch: %d", dir_str, pulse);

		if (button_event) {
			button_event = 0;
			
			switch (button_num) {
				case 1:
				angle += dir * 1.0;
				break;
				case 2:
				angle += dir * 5.0;
				break;
				case 3:
				dir = -dir;
				break;
				case 4:
				if (dir == 1) {
					angle = 180.0;
				}
				else {
					angle = 0.0;
				}
				break;
			}
			pulse = servo_move(angle);
		}
	}


	while (1) {

        servo_move(0);
        timer_waitMillis(1000);
        servo_move(45);
        timer_waitMillis(500);
        servo_move(90);
        timer_waitMillis(500);
        servo_move(135);
        timer_waitMillis(500);
        servo_move(180);
        timer_waitMillis(500);
	}
	
}
