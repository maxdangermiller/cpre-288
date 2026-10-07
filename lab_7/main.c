/**
 * @file main.c
 * @authors Max Miller, Miles Davis
 * 
 * @brief Created for CprE 288 Lab 7
 */

// #define PART_1
// #define PART_2
#define PART_3

#include <stdint.h>
#include <stdbool.h>
#include "timer.h"
#include "lcd.h"
#include "uart.h"
#include "driverlib/interrupt.h"


volatile char uart_data;
volatile char flag;


void main() {

	timer_init();
	lcd_init();

	uart_init();

#ifdef PART_1      // Receive and display text
    char str[23];
	int i = 0;
	char c;

	while (i < 20) {
		c = uart_receive();
		uart_sendChar(c);

		if (c == '\r' || c == '\n') {
			break;
		}

		str[i] = c;

		i++;
	};

	str[i] = '\0';

	lcd_printf("%s", str);
	

#endif

#ifdef PART_2      // Echo Received Character
	char str[23];
	int i = 0;
	char c;

	while (true) {
		i = 0;

		while (i < 20) {
			c = uart_receive();
			uart_sendChar(c);
	
			if (c == '\r' || c == '\n') {
				break;
			}
	
			str[i] = c;
	
			i++;
		};
		uart_sendChar('\r');
		uart_sendChar('\n');
	
		str[i] = '\0';
	
		lcd_printf("%s", str);
	}

#endif

#ifdef PART_3 // Receive characters using interrupts.
	char str[23];
	int i = 0;

	while (true) {

		if (flag) {
			uart_sendChar(uart_data);

			if (uart_data != '\r' && uart_data != '\n' && i < 20) {
				str[i] = uart_data;
				i++;
			}
			else {
				uart_sendChar('\r');
				uart_sendChar('\n');

				str[i] = '\0';
	
				lcd_printf("%s", str);

				i = 0;
			}

			flag = 0;
		}
	}
	
 
#endif
	return 0;
}

