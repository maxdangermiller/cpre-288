#include "ping.h"
#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <stdbool.h>
#include "Timer.h"

#define PB3_BITMASK 0b1000

static void send_ping(void);
void timer3b_ih(void);

volatile unsigned long time_1;
volatile unsigned long time_2;

volatile enum { PING_WAITING, PING_SENT, PING_T1_REC, PING_DONE} STATE;


void ping_init(void) {
	// Ping is connected to PB3


	SYSCTL_RCGCGPIO_R |= 0b10;				// Enable GPIO Port B's Clock

	while ((SYSCTL_PRGPIO_R & 0x8) == 0);	// Wait for clock to start


	GPIO_PORTB_DIR_R |= PB3_BITMASK;		// Set PB3 as output
	GPIO_PORTB_DEN_R |= PB3_BITMASK;		// Enable PB3
	GPIO_PORTB_AFSEL_R |= PB3_BITMASK;		// Set PB3 to be in alternate function mode temporarily
	GPIO_PORTB_PCTL_R &= ~0xF000;			// Reset PCTL for PB3
	GPIO_PORTB_PCTL_R |= 0x7000;			// Set PCTL to connect to timer 3b

	IntRegister(INT_TIMER3B, timer3b_ih);	// Set interrupt handler
	IntMasterEnable();

	NVIC_EN1_R |= 0x10;						// Enable interrupt
	SYSCTL_RCGCTIMER_R |= 0b1000;			// Enable Clock for Timer 3

	while ((SYSCTL_PRTIMER_R & 0x8) == 0);	// Wait for clock to start for timer 3

	// GPIO_PORTB_DATA_R &= ~PB3_BITMASK;		// Set PB3 to be LOW


	// Configure  Input Edge Time Mode on Timer 3B (Page 724 of datasheet)
	TIMER3_CTL_R &= ~0xD00;					// Clear Timer 3B Enable (TIMER_CTL_TBEN)
	TIMER3_CFG_R &= 0x4;					// Set to use 16bit timer
	TIMER3_TBMR_R = 0b0111;					// Configure Capture mode, Edge-Time mode, and count-up
	TIMER3_TBMR_R &= ~0b10000;
	TIMER3_TBPR_R |= 0xFF;
	TIMER3_CTL_R &= ~0xC00;					// set TB EVENT to both rising and falling edge triggers
	TIMER3_TBILR_R = 0xFFFF;				// Set initial timer value to 0
	TIMER3_CTL_R |= 0xD00;					// Re enable Timer 3B (TIMER_CTL_TBEN)


	STATE = PING_WAITING;
}

static void send_ping(void) {
	GPIO_PORTB_DATA_R &= ~PB3_BITMASK;		// LOW
	timer_waitMicros(10);					// Wait 10 micros
	GPIO_PORTB_DATA_R |= PB3_BITMASK;		// HIGH
	timer_waitMicros(5);					// Wait 5 micros
	GPIO_PORTB_DATA_R &= ~PB3_BITMASK;		// LOW

	TIMER3_ICR_R |= 0x400;                  // Clear any interrupt that may have triggered somehow
    GPIO_PORTB_AFSEL_R |= PB3_BITMASK;      // Enable alternate function
    TIMER3_IMR_R |= 0x400;                  // Enable interrupt
    TIMER3_CTL_R |= 0x100;                  // Enable Timer
}

int ping_read(void) {
	// TIMER3_CTL_R |= 0x100;                  // Enable Timer 3B
	// TIMER3_TBILR_R = 0;						// Set next time to 0
	// TIMER3_TBMR_R &= ~0x100;				// Tell the clock to reset the next time it can

	STATE = PING_WAITING;

	TIMER3_CTL_R &= ~0x100;					// Disable timer 3b
	TIMER3_IMR_R &= ~0x400;					// Disable interupt

	GPIO_PORTB_AFSEL_R &= ~PB3_BITMASK;     // Set PB3 to be in digital mode
	GPIO_PORTB_DIR_R |= PB3_BITMASK;        // Set PB3 as output

	printf("Sending Ping\n");
	STATE = PING_SENT;
	send_ping();
	printf("Ping sent\n");

	printf("Waiting for timer...\n");

	while (STATE != PING_DONE);
	printf("Time 1: %d\n", time_1);
	printf("Time 2: %d\n", time_2);

	int delta = time_2 - time_1;

	// (delta * 1/16MHz * 34000cm/sec) / 2 = dist (cm)
	return delta * 34000/16000000;
}


/**
 * Timer 3 Interupt handler
 */
void timer3b_ih(void) {
	// If this interupt isn't for ping, then disregard
	if (!(TIMER3_MIS_R & 0x400)) {
		return;
	}



	
	if (STATE == PING_SENT) {
		time_1 = TIMER3_TBR_R & 0xFFFFFF;
		STATE = PING_T1_REC;
	}

	else if (STATE == PING_T1_REC) {
		time_2 = TIMER3_TBR_R & 0xFFFFFF;
		STATE = PING_DONE;
	}
	// If we're not looking for an interupt, don't do anything

	TIMER3_ICR_R |= 0x400;				// Clear Interrupt
}
