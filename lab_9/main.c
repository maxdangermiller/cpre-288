#include "adc.h"
#include "Timer.h"
#include "lcd.h"
#include "cyBot_Scan_Cal.h"

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
