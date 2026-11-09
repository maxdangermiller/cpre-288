#include "servo.h"
#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include "Timer.h"
#include "math.h"
#include "lcd.h"

#define PB5_BITMASK 1 << 5
#define PWM_PERIOD 10000000 	// 10ms per period in ns
#define CLOCK_TICK_TIME 62.5	// 62.5ns per tick

const static int CLOCK_PERIOD = (int)(PWM_PERIOD / CLOCK_TICK_TIME);

static int servo_pulse_min = PWM_PULSE_MIN;
static int servo_pulse_max = PWM_PULSE_MAX;

static float servo_last_pos = 720.0; // Large out of bound number so we promote waiting the appropriate amount of time


void servo_init(void) {
	static uint8_t initialized = 0;

	if (initialized) { return; }

	SYSCTL_RCGCGPIO_R |= 0b10;				// Enable GPIO Port B's Clock
	while ((SYSCTL_PRGPIO_R & 0b10) == 0);	// Wait for Port B's clock to start

	SYSCTL_RCGCTIMER_R |= 0b10;			    // Enable Clock for Timer 1
	while ((SYSCTL_PRTIMER_R & 0b10) == 0);	// Wait for Timer 1's clock to start

	GPIO_PORTB_DEN_R |= PB5_BITMASK;		// Enable PB5
	GPIO_PORTB_AFSEL_R |= PB5_BITMASK;      // Set PB5 to use alternate function
	GPIO_PORTB_PCTL_R &= ~0xF00000;			// Reset PCTL for PB5
	GPIO_PORTB_PCTL_R |= 0x700000;			// Connect PB5 to T1CCP1


	// Datasheet page 725 and 744 in textbook
	TIMER1_CTL_R &= 0x100;					// Turn off Timer 1B
	TIMER1_CFG_R = 0x4;						// Use 16 bit timers
	TIMER1_TBMR_R |= 0b1010;				// Periodic Timer mode, PWM mode is enabled.
	TIMER1_TBMR_R &= ~0b1;					// Periodic Timer mode, PWM mode is enabled.
	TIMER1_CTL_R &= ~(1 << 14);				// PWM not inverted

	// TIMER1_TBILR_R = CLOCK_PERIOD;	// Period
	TIMER1_TBILR_R = CLOCK_PERIOD;		// Period
	TIMER1_TBPR_R =  CLOCK_PERIOD >> 16;
	// Load default position
	servo_move(90.0);

	TIMER1_CTL_R |= 0x100;					// Turn on Timer 1B

	initialized = 1;
	
}

int servo_move(float degrees) {
	// if 		(degrees < 0.0) 	{ degrees = 0.0; 	}
	// else if (degrees > 180.0) 	{ degrees = 180.0; 	}


	int pulse_length = (degrees / 180.0) * (servo_pulse_max - servo_pulse_min) + servo_pulse_min;
	int low = (PWM_PERIOD - pulse_length) / CLOCK_TICK_TIME;

	// Textbook page 766
	TIMER1_TBMATCHR_R = low; 	// Length of Low Pulse
	TIMER1_TBPMR_R = low >> 16;

	int pos_delta = (int)(fabs(servo_last_pos - degrees) * 8.0);
	servo_last_pos = degrees;

	timer_waitMillis(pos_delta + 50);

	return pulse_length;
}
