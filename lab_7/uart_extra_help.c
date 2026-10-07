/*
*
*   uart_extra_help.c
* Description: This is file is meant for those that would like a little
*              extra help with formatting their code, and followig the Datasheet.
*/

#include "uart.h"
#include "timer.h"
#define REPLACE_ME 0x00


void uart_init(int baud)
{
    SYSCTL_RCGCGPIO_R |= REPLACE_ME;      // enable clock GPIOB (page 340)
    SYSCTL_RCGCUART_R |= REPLACE_ME;      // enable clock UART1 (page 344)
    GPIO_PORTB_AFSEL_R = REPLACE_ME;      // sets PB0 and PB1 as peripherals (page 671)
    GPIO_PORTB_PCTL_R  = REPLACE_ME;       // pmc0 and pmc1       (page 688)  also refer to page 650
    GPIO_PORTB_DEN_R   = REPLACE_ME;        // enables pb0 and pb1
    GPIO_PORTB_DIR_R   = REPLACE_ME;        // sets pb0 as output, pb1 as input

    //compute baud values [UART clock= 16 MHz] 
    double fbrd;
    int    ibrd;

    fbrd = REPLACE_ME; // page 903
    ibrd = REPLACE_ME;
    fbrd = REPLACE_ME;

    UART1_CTL_R &= REPLACE_ME;      // disable UART1 (page 918)
    UART1_IBRD_R = REPLACE_ME;        // write integer portion of BRD to IBRD
    UART1_FBRD_R = REPLACE_ME;   // write fractional portion of BRD to FBRD
    UART1_LCRH_R = REPLACE_ME;        // write serial communication parameters (page 916) * 8bit and no parity
    UART1_CC_R   = REPLACE_ME;          // use system clock as clock source (page 939) 
    UART1_CTL_R |= REPLACE_ME;        // enable UART1

}

void uart_sendChar(char data)
{
   //TODO
   
}

char uart_receive(void)
{
 //TODO
 
}

void uart_sendStr(const char *data)
{
    //TODO
	
}

// _PART3


void uart_interrupt_init()
{
    // Enable interrupts for receiving bytes through UART1
    UART1_IM_R |= REPLACE_ME; //enable interrupt on receive - page 924

    // Find the NVIC enable register and bit responsible for UART1 in table 2-9
    // Note: NVIC register descriptions are found in chapter 3.4
    NVIC_EN0_R |= REPLACE_ME; //enable uart1 interrupts - page 104

    // Find the vector number of UART1 in table 2-9 ! UART1 is 22 from vector number page 104
    IntRegister(INT_UART1, REPLACE_ME); //give the microcontroller the address of our interrupt handler - page 104 22 is the vector number

}

void uart_interrupt_handler()
{
// STEP1: Check the Masked Interrup Status

//STEP2:  Copy the data 

//STEP3:  Clear the interrup   

}
