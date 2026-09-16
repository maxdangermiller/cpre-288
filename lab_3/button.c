/**
 * Button implementation for CprE 288 Lab 3
 * @file button.c
 * @headerfile button.h
 * 
 * @author Max Miller, Miles Davies
 * @date 09/09/2026
 */

//The buttons are on PORTE 3:0
// GPIO_PORTE_DATA_R -- Name of the memory mapped register for GPIO Port E, 
// which is connected to the push buttons
#include "button.h"

#define BIT_0 (1 << 0)
#define BIT_1 (1 << 1)
#define BIT_2 (1 << 2)
#define BIT_3 (1 << 3)
#define BIT_4 (1 << 4)
#define BIT_5 (1 << 5)
#define BIT_6 (1 << 6)
#define BIT_7 (1 << 7)


/**
 * Initialize PORTE and configure bits 0-3 to be used as inputs for the buttons.
 */
void button_init() {
	static uint8_t initialized = 0;

	//Check if already initialized
	if(initialized){
		return;
	}
	
	// Reading: To initialize and configure GPIO PORTE, visit pg. 656 in the 
	// Tiva datasheet.
	// See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=656
	
	// Follow steps in 10.3 for initialization and configuration. Some steps 
	// have been outlined below.
	
	// Ignore all other steps in initialization and configuration that are not 
	// listed below. You will learn more about additional steps in a later lab.

	// 1) Turn on PORTE system clock, do not modify other clock enables
	// See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=340
	SYSCTL_RCGCGPIO_R |= BIT_4;
	
	// 2) Set the buttons as inputs, do not modify other PORTE wires
	// See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=663
	GPIO_PORTE_DIR_R &= ~(BIT_3 + BIT_2 + BIT_1 + BIT_0);
	
	// 3) Enable digital functionality for button inputs, 
	//    do not modify other PORTE enables
	// See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=682
	GPIO_PORTE_DEN_R |= (BIT_3 + BIT_2 + BIT_1 + BIT_0);
	
	initialized = 1;
}



/**
 * Returns the position of the rightmost button being pushed.
 * @return 	the position of the right-most button being pushed. 
 * 			4 is the rightmost button, 1 is the leftmost button.  
 * 			0 indicates no button being pressed
 */
uint8_t button_getButton() {
    // See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=662

	// Check button 4
	if (!(GPIO_PORTE_DATA_R & BIT_3)) {
		return 4;
	}

	// Check button 3
	if (!(GPIO_PORTE_DATA_R & BIT_2)) {
		return 3;
	}

	// Check button 2
	if (!(GPIO_PORTE_DATA_R & BIT_1)) {
		return 2;
	}

	// Check button 1
	if (!(GPIO_PORTE_DATA_R & BIT_0)) {
		return 1;
	}
	
	return 0;
}





