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



int main(void) {
    button_init();
    lcd_init();

    // Part 4
    cyBot_uart_init();

    int btn, i;
    char* str = (char*)malloc(sizeof(char) * 11);

    while(1) {

        // Part 2/3
        btn = button_getButton();
        lcd_printf("Button: %d", btn);

        timer_waitMillis(100);

        // Part 4
        if (btn != 0) {
            sprintf(str, "Button %d has been pressed!\r\n", btn);

            for (i = 0; i < strlen(str); i++) {
                cyBot_sendByte(str[i]);
            }
        }




    }

}
