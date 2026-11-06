#include "ping.h"
#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <driverlib/interrupt.h>
#include "Timer.h"

#define PB3_BITMASK 0b1000
#define TIMER3B_CAPTURE_EVENT 0x400		// CBERIS / CBECINT bit in RIS, MIS, IMR, and ICR
#define TIMER_24BIT_MASK 0xFFFFFF		// 16-bit timer + 8-bit prescaler
#define SYSTEM_CLOCK_HZ 16000000.0f		// Timer 3 runs off the 16 MHz system clock
#define SPEED_OF_SOUND_CM_S 34000.0f	// 340 m/s, per the lab manual

static void send_trigger(void);
static void timer3b_handler(void);

static volatile unsigned long rising_time;
static volatile unsigned long falling_time;
static int overflow_count = 0;

static volatile enum { PING_IDLE, PING_WAIT_RISING, PING_WAIT_FALLING, PING_DONE } state = PING_IDLE;

void ping_init(void) {
	// Ping is connected to PB3

	SYSCTL_RCGCGPIO_R |= 0x2;				// Enable GPIO Port B's Clock
	while ((SYSCTL_PRGPIO_R & 0x2) == 0);	// Wait for Port B's clock to start

	SYSCTL_RCGCTIMER_R |= 0x8;				// Enable Clock for Timer 3
	while ((SYSCTL_PRTIMER_R & 0x8) == 0);	// Wait for Timer 3's clock to start

	GPIO_PORTB_DEN_R |= PB3_BITMASK;		// Enable PB3
	GPIO_PORTB_PCTL_R &= ~0xF000;			// Reset PCTL for PB3
	GPIO_PORTB_PCTL_R |= 0x7000;			// Connect PB3 to T3CCP1 (only used while AFSEL is set)

	// GPIO_PORTB_AFSEL_R &= ~PB3_BITMASK;		// Start in GPIO mode
	// GPIO_PORTB_DIR_R |= PB3_BITMASK;		// Set PB3 as output
	// GPIO_PORTB_DATA_R &= ~PB3_BITMASK;		// Set PB3 to be LOW

	// Configure Input Edge Time Mode on Timer 3B (Page 724 of datasheet)
	TIMER3_CTL_R &= ~0x100;					// Disable Timer 3B while configuring (TBEN)
	TIMER3_CFG_R = 0x4;						// Use 16 bit timers
	TIMER3_TBMR_R = 0b0111;					// Capture mode, Edge-Time mode, count-down
	TIMER3_CTL_R |= 0xC00;					// TBEVENT = both rising and falling edges
	TIMER3_TBPR_R = 0xFF;					// Prescaler extends the timer to 24 bits
	TIMER3_TBILR_R = 0xFFFF;				// Count down from the max value
	
	TIMER3_IMR_R |= 0x400;					// Keep the interrupt masked until a ping is sent
	TIMER3_ICR_R = 0x400;					// Clear any old capture event
	NVIC_EN1_R = (0b1 << (36 % 32));		// Enable Timer 3B in the NVIC (IRQ 36)
	
	IntRegister(0b110100, timer3b_handler);	// Set interrupt handler
	IntMasterEnable();						// Globally enable interrupts

	// TIMER3_CTL_R |= 0x100;					// Enable Timer 3B (TBEN)
}

/**
 * Sends a ping and blocks until the interrupt handler has captured both edges of the echo.
 * Returns the width of the echo pulse in clock cycles.
 */
int ping_read(void) {
	state = PING_IDLE;						// Reset state

	TIMER3_CTL_R &= ~0x100;					// Disable Timer 3B

	// TIMER3_IMR_R &= ~TIMER3B_CAPTURE_EVENT;	// Mask the interrupt while PB3 is an output

	// Switch to output
	GPIO_PORTB_AFSEL_R &= ~PB3_BITMASK;		// Set PB3 to GPIO mode
	GPIO_PORTB_DIR_R |= PB3_BITMASK;		// Set PB3 as output

	send_trigger();

	// Switch to input
	GPIO_PORTB_DIR_R &= ~PB3_BITMASK;		// Set PB3 as input
	GPIO_PORTB_AFSEL_R |= PB3_BITMASK;		// Give PB3 to Timer 3B (T3CCP1)

	TIMER3_ICR_R = 0x400;					// Clear the interrupt
	TIMER3_IMR_R |= 0x400;					// Unmask the interrupt
	TIMER3_CTL_R |= 0x100;					// Enable Timer 3B (TBEN)
	state = PING_WAIT_RISING;				// Set state to ping waiting on rising edge

	while (state != PING_DONE);				// Wait for the handler to catch both edges

	TIMER3_IMR_R &= ~0x400;					// Mask the interrupt again

	// Timer counts down, so the rising edge normally has the bigger value.
	// If it doesn't, the timer wrapped past 0 during the pulse.
	if (falling_time > rising_time) {
		overflow_count++;
	}

	// Timer counts down, so the rising edge has the bigger value.
	// Masking to 24 bits handles the timer wrapping during the pulse.
	return (rising_time - falling_time) & TIMER_24BIT_MASK;
}

/**
 * Returns how many times the timer has wrapped during an echo pulse.
 */
int ping_get_overflows(void) {
	return overflow_count;
}

/**
 * Converts a pulse width in clock cycles (16 MHz) to milliseconds.
 */
float ping_cycles_to_ms(int cycles) {
	return cycles / (SYSTEM_CLOCK_HZ / 1000.0f);
}

/**
 * Converts a pulse width in clock cycles (16 MHz) to distance in cm.
 * The sound travels to the object and back, so the round trip is halved.
 */
float ping_cycles_to_cm(int cycles) {
	return cycles * (SPEED_OF_SOUND_CM_S / SYSTEM_CLOCK_HZ) / 2.0f;
}

/**
 * Sends the low-high-low trigger pulse on PB3, then hands PB3 to Timer 3B.
 */
static void send_trigger(void) {
	// Send Pulse
	GPIO_PORTB_DATA_R &= ~PB3_BITMASK;		// LOW
	timer_waitMicros(20);					// Wait 20 micros
	GPIO_PORTB_DATA_R |= PB3_BITMASK;		// HIGH
	timer_waitMicros(5);					// Wait 5 micros
	GPIO_PORTB_DATA_R &= ~PB3_BITMASK;		// LOW
}

/**
 * Timer 3B interrupt handler. Saves the captured time of each edge of the echo.
 */
static void timer3b_handler(void) {
	// If this interrupt isn't a capture event, disregard it
	// if (!(TIMER3_MIS_R & 0x400)) { 		// 0x400 = TIMER3B_CAPTURE_EVENT
	// 	return;
	// }

	TIMER3_ICR_R = 0x400;				// Clear the capture event

	if (state == PING_WAIT_RISING) {
		rising_time = TIMER3_TBR_R & TIMER_24BIT_MASK;
		state = PING_WAIT_FALLING;
	}
	else if (state == PING_WAIT_FALLING) {
		falling_time = TIMER3_TBR_R & TIMER_24BIT_MASK;
		state = PING_DONE;
	}
}
