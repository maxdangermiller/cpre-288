/**
 * @file main.c
 * 
 * CprE 288 lab 6
 */

#include "button.h"
#include "timer.h"
#include "lcd.h"

#include "cyBot_uart.h"  // Functions for communiticate between CyBot and Putty (via UART)
						 // PuTTy: Buad=115200, 8 data bits, No Flow Control, No Party,  COM1

#include "cyBot_Scan.h"  // For scan sensors 

#include "uart_utils.h"
#include "cyBot_Scan_Cal.h"

#define UART1_PCTLB_BITMASK 0b00010001
#define UART1_BITMASK 0b00000011



// Defined in button.c : Used to communicate information between the
// the interupt handeler and main.
extern volatile int button_event;
extern volatile int button_num;


int main(void) {
	button_init();
	timer_init();
	lcd_init();

	init_button_interrupts();
	
	cyBot_uart_init_clean();  // Clean UART initialization, before running your UART GPIO init code

	// Complete this code for configuring the  (GPIO) part of UART initialization

	//Helpful: // See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=902

	//Turn on PORTB system clock
	// See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=340
	SYSCTL_RCGCGPIO_R |= 0b000010;
	timer_waitMillis(1);

	// We want to use UART1 TX & RX 
	// Table 10-2: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=650
	// Table 10-3: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=657

	// Register Information
	// AFSEL Register:  https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=671
	// PCTL Register:   https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=688
	// DEN Register:    https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=682
	// DIR Register:    https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=663
	
	GPIO_PORTB_AFSEL_R |= 0b00000011;

	GPIO_PORTB_PCTL_R &= ~UART1_PCTLB_BITMASK;
	GPIO_PORTB_PCTL_R |= UART1_PCTLB_BITMASK;

	GPIO_PORTB_DEN_R |= UART1_BITMASK;

	// RX is an input, and TX is an output
	GPIO_PORTB_DIR_R &= ~(0b00000000);
	GPIO_PORTB_DIR_R |= 0b00000001;
	
	// Completes the UART device initialization part of configuration
	cyBot_uart_init_last_half();
	
	// Initialze scan sensors
	cyBot_init_Scan_cust(0b111);


	// For Part 3a
	// lcd_printf("IR CALIBRATION");
	// cyBot_IR_Calibrate();


	char str[50];
	cyBOT_Scan_t data[3];
	
	// lcd_printf("Button: ");
	lcd_printf("Distance: ");
		
	while(1) {
	
		if (button_event) {
			// lcd_printf("Button: %d", button_num);
			// sprintf(str, "Button: %d\r\n", button_num);
			// cyBot_sendString(str);
			
			cyBOT_Scan(90, &data[0]);
			cyBOT_Scan(90, &data[1]);
			cyBOT_Scan(90, &data[2]);
			
			double dist = 0.0;
			double act = 0.0;
			
			dist += cyBot_est_IR_dist((double)data[0].IR_raw_val);
			dist += cyBot_est_IR_dist((double)data[1].IR_raw_val);
			dist += cyBot_est_IR_dist((double)data[2].IR_raw_val);

			act += (double)data[0].sound_dist;
			act += (double)data[1].sound_dist;
			act += (double)data[2].sound_dist;
			
			dist = dist / 3;
			act = act / 3;
			
			sprintf(str, "Distance: %lf\r\n\r\nSound: %lf\r\n", dist, act);
			lcd_printf("Distance: %lf\n\nSound: %lf", dist, act);
			cyBot_sendString(str);

			timer_waitMillis(100);
			button_event = 0;
		}
		// lcd_printf("Button: %d (wrong)", button_getButton());
	
	}
	
}
