#ifndef ADC_H
#define ADC_H

#define EN_CALIBRATE
#define EN_HARDWARE_SAMPLING

#include "cybot.h"

#ifdef USE_CYBOT_17

// RMSE: 70.07
#define IR_DIST_A_COF 17700
#define IR_DIST_B_COF -0.8609

#elif defined(USE_CYBOT_03)

// RMSE: 1.595
#define IR_DIST_A_COF 72910
#define IR_DIST_B_COF -1.101

#else

#define IR_DIST_A_COF 19640
#define IR_DIST_B_COF -0.8609 // Default value

#endif


void adc_init(void);
int adc_read(void);

float adc_get_dist(float ir_dist);


#ifdef EN_CALIBRATE

void adc_calibrate(void);

#endif

#endif
