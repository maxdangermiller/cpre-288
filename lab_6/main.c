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
    YSCTL_RCGCGPIO_R |= FIXME;
    timer_waitMillis(1);

    GPIO_PORTB_AFSEL_R |= FIXME;
    GPIO_PORTB_PCTL_R &= FIXME;     // Force 0's in the disired locations
    GPIO_PORTB_PCTL_R |= FIXME;     // Force 1's in the disired locations
    GPIO_PORTB_DEN_R |= FIXME;
    GPIO_PORTB_DIR_R &= FIXME;      // Force 0's in the disired locations
    GPIO_PORTB_DIR_R |= FIXME;      // Force 1's in the disired locataions
    
    // (Uncomment ME for UART init part of lab) cyBot_uart_init_last_half();  // Completes the UART device initialization part of configuration
	
	// Initialze scan sensors
    // cyBOT_init_Scan();



	lcd_printf("Button: ");
		
	while(1) {
	
		if (button_event) {
			lcd_printf("Button: %d", button_num);
		}
		// lcd_printf("Button: %d (wrong)", button_getButton());
	
	}
	
}
