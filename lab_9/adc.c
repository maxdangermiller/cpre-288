#include "adc.h"

#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <stdbool.h>

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
	SYSCTL_RCGCADC_R |= 0b01;           // Enable ADC0's CLOCK
	SYSCTL_RCGCGPIO_R |= 0b10;          // Enable GPIO Port B's Clock

	GPIO_PORTB_AFSEL_R |= 0b00010000;	// Select Alternate Function for PB4
	GPIO_PORTB_PCTL_R &= ~0xF0000;		// Reset PCTL Register for Port B Pin 4

	GPIO_PORTB_DEN_R &= ~0b00010000;	// Disable digital function on Port B Pin 4
	
	GPIO_PORTB_AMSEL_R = 0b00010000;	// Disable all analog function on port b except for pin 4


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


	ADC0_ACTSS_R &= ~0b1000; //disable Sample Sequencer 3

	ADC0_EMUX_R &= ~0xF000; //Select software trigger by clearing bits 12-15

	ADC0_SSMUX3_R &= ~0xF; //set input channel 0

	ADC0_SSCTL3_R |= 0x6; //configure sample control bits

	ADC0_ACTSS_R |= 0b1000; //enable Sample Sequencer 3 (step 7)
}

int adc_read(void) {

	/**
	 * Sample Sequence Results found in registers 17-20, FIFO 0-3 respectively (see page 860)
	 * Let's use 3
	 */

	
	ADC0_PSSI_R |= 0b1000; //trigger conversion in 3

	while(!(ADC0_RIS_R & 0b1000)){
		//do nothing
	}

	int result = (ADC0_SSFIFO3_R & 0xFFF);

	ADC0_ISC_R &= ~0b1000;

	return result;

}
