/**
 * @file uart.c
 * @authors Max Miller, Miles Davis
 * @date 10/07/2026
 */

#include "uart.h"
#include "driverlib/interrupt.h"
#include <stdint.h>
#include <stdbool.h>

#define BAUD_RATE 115200
#define CLOCK_RATE 16000000
#define DATA_BITS 0x3 		// 8-bit UART Word Length (page 916)
#define STOP_BITS 1
#define PARITY 0

volatile char uart_data;
volatile char flag;

void uart_interrupt_init();
void uart_interrupt_handler();

void uart_init(void){
	//Turn on PORTB system clock
	// See: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=340
	SYSCTL_RCGCGPIO_R |= 0b000010;
	timer_waitMillis(1);

	// We want to use UART1 TX & RX 
	// Table 10-2: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=650
	// Table 10-3: https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=657

	// Register Information
	// AFSEL:  		https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=671
	// RCGCUART:	https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=344
	// PCTL:   		https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=688
	// DEN:    		https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=682
	// DIR:    		https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=663
	
	GPIO_PORTB_AFSEL_R |= 0b00000011;	// enable clock GPIOB
	SYSCTL_RCGCUART_R |= 0b10;      	// enable clock UART1

	// GPIO_PORTB_PCTL_R &= ~0b00010001;	// sets PB0 and PB1
	GPIO_PORTB_PCTL_R |= 0x11;	// pmc0 and pmc1

	GPIO_PORTB_DEN_R |= 0b00000011;		

	GPIO_PORTB_DIR_R |= 0b00000001;		// sets pb0 as output	(TX)
	GPIO_PORTB_DIR_R &= ~(0b0000010);	// sets pb1 as input	(RX)
	
	double fbrd;						// Float baud rate
    int    ibrd;						// Int baud rate

	// UART BAUD RATE: 	https://class.ece.iastate.edu/cpre288/resources/docs/Tiva_TM4C123GH6PM_datasheet.pdf#page=903
	fbrd = (double)CLOCK_RATE / (16.0 * BAUD_RATE);
    ibrd = (int)(fbrd);
    fbrd = (fbrd - ibrd) * 64 + 0.5;

	int line_ctrl = (DATA_BITS << 5) + (PARITY << 1);

	UART1_CTL_R &= ~0b1;      			// disable UART1 (page 918)
    UART1_IBRD_R = ibrd;        		// write integer portion of BRD to IBRD
    UART1_FBRD_R = (int)fbrd;   		// write fractional portion of BRD to FBRD
    UART1_LCRH_R = line_ctrl;        	// write serial communication parameters (page 916) * 8bit and no parity
    UART1_CC_R   = 0x0;          		// use system clock as clock source (page 939) 
    UART1_CTL_R |= 0b1;        			// enable UART1

    uart_interrupt_init();
}

void uart_sendChar(char data) {
	while((UART1_FR_R & 0x20) != 0);

	UART1_DR_R = data;
}

char uart_receive(void) {
	while ((UART1_FR_R & 0x10) != 0);

	if (UART1_DR_R & 0b111100000000) {
		// And error occured
	    printf("We got an error me bois");
		return '\0';
	}

	return (char)(UART1_DR_R & 0xFF);
}

void uart_sendStr(const char *data) {
	// While not equal to null
	while(*data != '\0') {
		uart_sendChar(*data);
		data++;
	}
}

void uart_interrupt_init() {
    // Enable interrupts for receiving bytes through UART1
    UART1_IM_R |= 0b10000; //enable interrupt on receive - page 924

    // Find the NVIC enable register and bit responsible for UART1 in table 2-9
    // Note: NVIC register descriptions are found in chapter 3.4
    NVIC_EN0_R |= 0b1000000; //enable uart1 interrupts - page 104

    // Find the vector number of UART1 in table 2-9 ! UART1 is 22 from vector number page 104
    IntRegister(INT_UART1, uart_interrupt_handler); //give the microcontroller the address of our interrupt handler - page 104 22 is the vector number

}

void uart_interrupt_handler() {
	// STEP1: Check the Masked Interrup Status
	if ((UART1_MIS_R & 0b10000) == 0) {
		return;
	}

	//STEP2:  Copy the data 
	uart_data = uart_receive();
	flag = 1;
	
	//STEP3:  Clear the interrup   
	UART1_ICR_R &= ~0b10000;

}
