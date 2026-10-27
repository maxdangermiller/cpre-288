#ifndef ADC_H
#define ADC_H

// #define EN_CALIBRATE
// #define EN_HARDWARE_SAMPLING


void adc_init(void);
int adc_read(void);

float adc_get_dist(int ir_dist);


#ifdef EN_CALIBRATE

void adc_calibrate(void);

#endif

#endif
