/**
 * @brief Main File for Lab 11
 * @author Max Miller, Miles Davies
 */
#include "servo.h"
#include "Timer.h"

int main(void) {
    timer_init();
    lcd_init();
    servo_init();

    servo_move(0.0);


    /*
    int i = 0;

    while (1) {
        if (i++ > 180) {
            i = 0;
        }

        servo_move(i);

        timer_waitMillis(10);
    }
    */
}
