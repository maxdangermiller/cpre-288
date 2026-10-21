#include "adc.h"
#include "timer.h"
#include "lcd.h"

int main()
{
    timer_init();
    lcd_init();
    adc_init();
    while(true){
        lcd_printf("R0: %d", adc_read());
        timer_waitMillis(100);
    }
    
    /* code */
    return 0;




}
