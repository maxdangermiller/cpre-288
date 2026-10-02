/**
 * Button implementation for CprE 288 Lab 6
 * @file button.c
 * @headerfile button.h
 * 
 * @author Max Miller, Miles Davies
 * @date 09/30/2026
 */
 


//The buttons are on PORTE 3:0
// GPIO_PORTE_DATA_R -- Name of the memory mapped register for GPIO Port E, 
// which is connected to the push buttons
#include "button.h"

// Global varibles
volatile int button_event;
volatile int button_num;

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
	SYSCTL_RCGCGPIO_R |= 0b10000;
	
	// 2) Set the buttons as inputs, do not modify other PORTE wires
	// See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=663
	GPIO_PORTE_DIR_R &= ~(0b1111);
	
	// 3) Enable digital functionality for button inputs, 
	//    do not modify other PORTE enables
	// See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=682
	GPIO_PORTE_DEN_R |= (0b1111);
	
	initialized = 1;
}


/**
 * Initialize and configure PORTE interupts
 */
void init_button_interrupts() {

	const unsigned int pin_bitmask = 0b1111;

    // 1) Mask the bits for pins 0-3
	// https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=667
    GPIO_PORTE_IM_R &= ~pin_bitmask;

    // 2) Set pins 0-3 to use edge sensing
	// https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=664
    GPIO_PORTE_IS_R &= ~pin_bitmask;

    // 3) Set pins 0-3 to use both edges. We want to update the LCD
    //    when a button is pressed, and when the button is released.
	// https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=665
    GPIO_PORTE_IBE_R |= pin_bitmask;
	
    // 4) Clear the interrupts
	// https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=670
    GPIO_PORTE_ICR_R = pin_bitmask;
	
    // 5) Unmask the bits for pins 0-3
	// https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=667
    GPIO_PORTE_IM_R |= pin_bitmask;

    // 6) Enable GPIO port E interrupt
    // https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=142
    NVIC_EN0_R |= 0b10000;

    // Bind the interrupt to the handler.
    IntRegister(INT_GPIOE, gpioe_handler);
}


/**
 * Interrupt handler -- executes when a GPIO PortE hardware event occurs (i.e., for this lab a button is pressed)
 */
void gpioe_handler() {
    // Clear interrupt status register
    GPIO_PORTE_ICR_R = 0b1111;

    button_event = true;
    button_num = button_getButton();
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
	if (!(GPIO_PORTE_DATA_R & 0b1000)) {
		return 4;
	}

	// Check button 3
	if (!(GPIO_PORTE_DATA_R & 0b0100)) {
		return 3;
	}

	// Check button 2
	if (!(GPIO_PORTE_DATA_R & 0b0010)) {
		return 2;
	}

	// Check button 1
	if (!(GPIO_PORTE_DATA_R & 0b0001)) {
		return 1;
	}
	
	return 0;
}
