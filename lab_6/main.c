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

    //Helpful: // See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=902

    //Turn on PORTB system clock
    // See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=340
    SYSCTL_RCGCGPIO_R |= 0b0010;
    timer_waitMillis(1);

    // See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=671
    GPIO_PORTB_AFSEL_R |= FIXME;
    // See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=688
    GPIO_PORTB_PCTL_R &= FIXME;     // Force 0's in the desired locations
    GPIO_PORTB_PCTL_R |= FIXME;     // Force 1's in the desired locations
    // See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=682
    GPIO_PORTB_DEN_R |= FIXME;
    // See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=663
    GPIO_PORTB_DIR_R &= FIXME;      // Force 0's in the desired locations
    GPIO_PORTB_DIR_R |= FIXME;      // Force 1's in the desired locations
    
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
