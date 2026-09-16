#include "open_interface.h"
#include "movement.h"
#include <stdio.h>
#include <string.h>
#include <Timer.h>
#include <math.h>
#include "cyBot_uart.h"
#include "lcd.h"

void turn_test();

void step_2();
void step_2_2();
void turn_test();
void step_3();
void step_4();
void step_5();

/**
 * main.c
 * @author Max Miller
 */
int main(void) {
    // step_2();
    // step_2_2();
    // step_3();
    // step_4();
    // step_5();
}


void step_2() {
    oi_t *sensor_data = oi_alloc();

    oi_init(sensor_data);

    double sum = 0;

    oi_setWheels(500, 500); // move forward; full speed

    while (sum < 1000) {
        oi_update(sensor_data);
        sum += sensor_data->distance;
    }

    oi_setWheels(0, 0); // stop
    oi_free(sensor_data);

}

void step_2_2() {
    oi_t *sensor = oi_alloc();

    oi_init(sensor);

    move_forward(sensor, 100);

    oi_free(sensor);

}

void turn_test() {
    timer_init();

    oi_t *sensor = oi_alloc();
    oi_init(sensor);

    turn_cw(sensor, 90);
    timer_waitMillis(2000);
    turn_cw(sensor, 90);
    timer_waitMillis(2000);
    turn_cw(sensor, 90);
    timer_waitMillis(2000);
    turn_cw(sensor, 90);
    timer_waitMillis(10000);

    turn_ccw(sensor, 90);
    timer_waitMillis(2000);
    turn_ccw(sensor, 90);
    timer_waitMillis(2000);
    turn_ccw(sensor, 90);
    timer_waitMillis(2000);
    turn_ccw(sensor, 90);
    timer_waitMillis(2000);

    oi_free(sensor);
}

void step_3() {
    oi_t *sensor = oi_alloc();

    oi_init(sensor);

    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            move_forward(sensor, 50);
            turn_cw(sensor, 90);
        }

    }


    oi_free(sensor);
}

void step_4() {
    oi_t *sensor = oi_alloc();

    oi_init(sensor);

    move_forward(sensor, 200);

    oi_free(sensor);
}

void step_5() {
    oi_t *sensor = oi_alloc();

    oi_init(sensor);
    lcd_init();
    cyBot_uart_init();

    char c;
    int i;
    char* str = (char*)malloc(sizeof(char) * 11);

    while (true) {
        c = (char)cyBot_getByte();

        lcd_putc(c);

        if (c == 'w') {
            move_forward(sensor, 10);
        }
        else if (c == 'a') {
            turn_ccw(sensor, 90);
        }
        else if (c == 's') {
            move_backward(sensor, 10);
        }
        else if (c == 'd') {
            turn_cw(sensor, 90);
        }

        else if (c == 'm') {
            sprintf(str, "Got an %c\r\n", c);

            for (i = 0; i < strlen(str); i++) {
                cyBot_sendByte(str[i]);
            }
        }

    }
    
    oi_free(sensor);
}
