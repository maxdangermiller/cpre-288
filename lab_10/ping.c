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
	TIMER3_ICR_R |= 0x600;					// Enable Timer 3B
	TIMER3_IMR_R |= 0x600;					// CHECK
	// NVIC_PRI21_R							// Page 155 in Datasheet

	
	// Configure  Input Edge Time Mode on Timer 3B (Page 724 of datasheet)
	TIMER3_CTL_R &= ~0x100;					// Clear Timer 3B Enable (TIMER_CTL_TBEN)
	TIMER3_CFG_R &= 0x4;					// Set to use 16bit timer
	TIMER3_TBMR_R = 0b10111;				// Configure Capture mode, Edge-Time mode, and count-up
	TIMER3_CTL_R &= ~0xC00;					// set TB EVENT to both rising and falling edge triggers
	TIMER3_TBILR_R = 0;						// Set initial timer value to 0
	// TIMER3_IMR_R |= (0b1 << 10);			// Enable interrupt
	TIMER3_CTL_R |= 0x100;					// Re enable Timer 3B (TIMER_CTL_TBEN)



}

int ping_read(void) {
	TIMER3_TBILR_R = 0;						// Set next time to 0
	TIMER3_TBMR_R &= ~0x100;				// Tell the clock to reset the next time it can

    GPIO_PORTB_DIR_R |= PB3_BITMASK;		// Set PB3 as output

	GPIO_PORTB_DATA_R &= ~PB3_BITMASK;		// LOW
	timer_waitMicros(5);					// Wait 5 micros
	GPIO_PORTB_DATA_R |= PB3_BITMASK;		// HIGH
	timer_waitMicros(5);					// Wait 5 micros
	GPIO_PORTB_DATA_R &= ~PB3_BITMASK;		// LOW
	
	GPIO_PORTB_DIR_R &= ~PB3_BITMASK;		// Set PB3 as input

	GPIO_PORTB_AFSEL_R |= PB3_BITMASK;
	GPIO_PORTB_PCTL_R &= ~0xF000;
	GPIO_PORTB_PCTL_R |= 0x7000;

	int time1, time2; 

	while (!(TIMER3_RIS_R & 0xC00));		// Poll the CnERIS bit in the GPTMRIS register until there's an edge
	
	TIMER3_ICR_R |= 0xC00;					// Clear Interrupt
	time1 = TIMER3_TBR_R;					// Save current time to time1

	while (!(TIMER3_RIS_R & 0xC00));		// Poll the CnERIS bit in the GPTMRIS register until there's an edge
	
	TIMER3_ICR_R |= 0xC00;					// Clear Interrupt
	time2 = TIMER3_TBR_R;					// Save current time to time2

	int delta = time2 - time1;

	// (delta * 1/16MHz * 34000cm/sec) / 2 = dist (cm)
	return delta * 34000/16000000;
}
