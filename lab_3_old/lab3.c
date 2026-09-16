/**
 * lab3.c
 * 
 * Main file for CprE 288 lab 3
 *
 * @author Max Miller, Miles Davies
 * @date 09/09/2026
 */

#include "button.h"
#include "Timer.h"
#include "lcd.h"
#include "cyBot_uart.h"  // Functions for communiticate between CyBot and Putty (via UART)
                         // PuTTy: Buad=115200, 8 data bits, No Flow Control, No Party,  COM1


#warning "Possible unimplemented functions"
#define REPLACEME 0



int main(void) {
	button_init();
	lcd_init();
	// cyBot_uart_init();
	
	// YOUR CODE HERE
	
	
	while(1) {
	
	    // YOUR CODE HERE
	    lcd_clear();
	    lcd_printf("Button: %d", button_getButton());


	
	}
	
}
