#include "adc.h"

#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/**
 * @brief Initialize ADC for IR sensor
 * 
 */
void adc_init(void) {
	//  AIN10 on PB4 - using ADC0 and SS0

	/**
	 * 1.   Enable the ADC clock using the RCGCADC register (see page 352).
	 * 
	 * 2.   Enable the clock to the appropriate GPIO modules via the RCGCGPIO register (see page 340).
	 *      To find out which GPIO ports to enable, refer to â€œSignal Descriptionâ€� on page 801.
	 * 
	 * 3.   Set the GPIO AFSEL bits for the ADC input pins (see page 671). To determine which GPIOs to
	 *      configure, see Table 23-4 on page 1344.
	 * 
	 * 4.   Configure the AINx signals to be analog inputs by clearing the corresponding DEN bit in the
	 *      GPIO Digital Enable (GPIODEN) register (see page 682).
	 * 
	 * 5.   Disable the analog isolation circuit for all ADC input pins that are to be used by writing a 1 to
	 *      the appropriate bits of the GPIOAMSEL register (see page 687) in the associated GPIO block.
	 * 
	 * 6.   If required by the application, reconfigure the sample sequencer priorities in the ADCSSPRI
	 *      register. The default configuration has Sample Sequencer 0 with the highest priority and Sample
	 *      Sequencer 3 as the lowest priority.
	 */
	SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R0;           // Enable ADC0's CLOCK
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGC2_GPIOB;          // Enable GPIO Port B's Clock

	GPIO_PORTB_AFSEL_R |= 0b00010000;	// Select Alternate Function for PB4
	GPIO_PORTB_PCTL_R &= ~GPIO_PCTL_PB4_M;		// Reset PCTL Register for Port B Pin 4

	GPIO_PORTB_DEN_R &= ~0b00010000;	// Disable digital function on Port B Pin 4
	
	GPIO_PORTB_AMSEL_R = 0b00010000;	// Disable all analog function on port b except for pin 4

	GPIO_PORTB_ADCCTL_R = 0x0;			// Set Port B to use ADC0

	/**
	 * 
	 * Configuration of the sample sequencers (13.4.2 pg 818)
	 * 
	 * 
	 * 1. 	Ensure that the sample sequencer is disabled by clearing the corresponding 
	 * 		ASENn bit in the ADCACTSS register (page 821). Programming of the sample sequencers is allowed 
	 * 		without having them enabled. Disabling the sequencer during programming prevents 
	 * 		erroneous execution if a trigger event were to occur during the configuration process.
	 * 
	 * 2. 	Configure the trigger event for the sample sequencer in the ADCEMUX register.
	 * 
	 * 3. 	When using a PWM generator as the trigger source, use the ADC Trigger Source Select 
	 * 		(ADCTSSEL) register to specify in which PWM module the generator is located. The default 
	 * 		register reset selects PWM module 0 for all generators.
	 * 
	 * 4. 	For each sample in the sample sequence, configure the corresponding input source in the
	 * 		ADCSSMUXn register.
	 * 
	 * 5. 	For each sample in the sample sequence, configure the sample control bits in the corresponding 
	 * 		nibble in the ADCSSCTLn register. When programming the last nibble, ensure that the END bit 
	 * 		is set. Failure to set the END bit causes unpredictable behavior.
	 * 
	 * 6. 	If interrupts are to be used, set the corresponding MASK bit in the ADCIM register.
	 * 
	 * 7. 	Enable the sample sequencer logic by setting the corresponding ASENn bit in the ADCACTSS 
	 * 		register.
	 */


	ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN0; 		//disable Sample Sequencer 0

	// ADC0_EMUX_R &= ~ADC_EMUX_EM0_M; 		//Select software trigger
	ADC0_EMUX_R &= ~ADC_EMUX_EM0_PROCESSOR; // Select default trigger

	ADC0_SSMUX0_R &= ~0x0; 					// set first sample to come from channel 0

	ADC0_SSCTL0_R |= ADC_SSCTL0_END0; 		// Stop after first sample (since theres only ch0 that we're reading from)

#ifdef EN_HARDWARE_SAMPLING
	ADC0_SAC_R = ADC_SAC_AVG_16X;			// Setup hardware averaging (See page 847)
#endif

	ADC0_ACTSS_R |= ADC_ACTSS_ASEN0; 		//enable Sample Sequencer 0 (step 7)
}

/**
 * @brief Read IR sensor value using ADC
 * 
 * @return int IR sensor value in 12 bits (0-4095)
 */
int adc_read(void) {

	/**
	 * Sample Sequence Results found in registers 17-20, FIFO 0-3 respectively (see page 860)
	 * Let's use 3
	 */

	
	ADC0_PSSI_R |= ADC_PSSI_SS0; // trigger conversion in SS0

	while((ADC0_RIS_R & ADC_PSSI_SS0) == 0);

	int result = ADC0_SSFIFO0_R;

	ADC0_ISC_R &= ADC_ISC_IN0;

	return result;
}


/**
 * @brief Get Distance conversion for IR
 * 
 * @param ir_dist measured IR distance (0-4095)
 * @return float distance
 */
float adc_get_dist(int ir_dist) {
	// y = ax^b
    // a = 3076000
    // b = -1.588
    // RMSE: 1.393
    return 3076000.0 * pow(ir_dist, -1.588);
}


#ifdef EN_CALIBRATE

#include "cyBot_Scan.h"
#include "button.h"
#include "uart.h"
#include "lcd.h"
#include "Timer.h"
#include "servo.h"

extern volatile int button_event;

/**
 * Calibrate IR Distance
 */
void adc_calibrate() {
	static const int DIST_COUNT = 7;		// Distances
	static const int SAMP_COUNT = 3;		// Samples

    cyBOT_Scan_t calibration_data[DIST_COUNT * SAMP_COUNT];
	
    static char str[50];
    cyBOT_Scan_t data;

    int i,j;

    sprintf(str, "\r\nStarting IR Calibration\r\n\r\n");
    uart_sendStr(str);

	// Don't enable IR, since this file handles it
	cyBOT_init_Scan(0b011);

	left_calibration_value = SERVO_LEFT_CAL;
	right_calibration_value = SERVO_RIGHT_CAL;

	uart_init();
	timer_init();
	lcd_init();

    // Go ahead and move the servo to 90 degrees
    cyBOT_Scan(90, &data);

    for (i = 0; i < DIST_COUNT; i++) {
        uart_sendStr("Place IR Sensor At a new distance between 9cm and 50cm (press and button to measure)\n\r");

        while (!button_event) {
			cyBOT_Scan(90, &data);
			lcd_printf("Cur Dist of %6.2lfcm", data.sound_dist);

			sprintf(str, "\rCurrently measuring distance of %6.2lfcm", data.sound_dist);
			uart_sendStr(str);
		}
		
		uart_sendChar('\n');
		uart_sendChar('\r');


        for (j = 0; j < SAMP_COUNT; j++) {
            cyBOT_Scan(90, &data);

            calibration_data[SAMP_COUNT * i + j].IR_raw_val = adc_read();
            calibration_data[SAMP_COUNT * i + j].sound_dist = data.sound_dist;
        }

        timer_waitMillis(50); // Debounce
        button_event = 0;
    }

    sprintf(str, "All Data Collected. Sending Data in csv format.\r\n\r\n");
    uart_sendStr(str);

    sprintf(str, "sample #,ir,sound\r\n");
    uart_sendStr(str);

    for (i = 0; i < 21; i++) {
        sprintf(str, "%2d, %4d, %6.1f\r\n", i, calibration_data[i].IR_raw_val, calibration_data[i].sound_dist);
        uart_sendStr(str);
    }

    sprintf(str, "\r\nCalibration Finished.\r\n");
    uart_sendStr(str);

}

#endif
