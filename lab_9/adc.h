#ifndef ADC_H
#define ADC_H

#define EN_CALIBRATE
#define EN_HARDWARE_SAMPLING

#include "cybot.h"

#ifdef USE_CYBOT_17

#define IR_DIST_COF 17670   // RMSE: 24.18

#else

#define IR_DIST_COF 17700 // Default value

#endif


void adc_init(void);
int adc_read(void);

float adc_get_dist(float ir_dist);


#ifdef EN_CALIBRATE

void adc_calibrate(void);

#endif

#endif
