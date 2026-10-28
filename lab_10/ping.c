#include "ping.h"
#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <stdbool.h>
#include "Timer.h"

#define PB3_BITMASK 0b1000

void ping_init(void) {
	// Ping is connected to PB3


	SYSCTL_RCGCGPIO_R |= 0b10;				// Enable GPIO Port B's Clock
	GPIO_PORTB_DEN_R |= PB3_BITMASK;		// Enable PB3
	GPIO_PORTB_AFSEL_R &= ~PB3_BITMASK;		// Set PB3 to be in digital mode
	GPIO_PORTB_DIR_R |= PB3_BITMASK;		// Set PB3 as output
	GPIO_PORTB_DATA_R &= ~PB3_BITMASK;		// Set PB3 to be LOW

	// TEXTBOOK Page 762
	TIMER3_CTL_R &= ~0x100;					// Disable Timer B
	TIMER3_CFG_R = 0;						// Set to 32-
	TIMER3_TBMR_R = 0x7;					// Capture Mode
	TIMER3_CTL_R &= ~0xC00;					// TAEVENT is rising edge trigger
	TIMER3_ICR_R |= 0x600;					// 
	TIMER3_IMR_R |= 0x600;					// CHECK
	// TODO: Continue and finish

	
}

int ping_read(void) {
    GPIO_PORTB_DIR_R |= PB3_BITMASK;        // Set PB3 as output

	GPIO_PORTB_DATA_R &= ~PB3_BITMASK;		// LOW
	timer_waitMicros(5);					// Wait 5 micros
	GPIO_PORTB_DATA_R |= PB3_BITMASK;		// HIGH
	timer_waitMicros(5);					// Wait 5 micros
	GPIO_PORTB_DATA_R &= ~PB3_BITMASK;      // LOW
	
	GPIO_PORTB_DIR_R &= ~PB3_BITMASK;        // Set PB3 as input

	GPIO_PORTB_AFSEL_R |= PB3_BITMASK;
	GPIO_PORTB_PCTL_R &= ~0xF000;
	GPIO_PORTB_PCTL_R |= 0x7000;
}
