#ifndef SERVO_H
#define SERVO_H

#include "cybot.h"

void servo_init(void);
int servo_move(float degrees);

#ifdef USE_CYBOT_01

#define PWM_PULSE_MIN 232750
#define PWM_PULSE_MAX 1225000

#elif defined(USE_CYBOT_03)

#define PWM_PULSE_MIN 259000
#define PWM_PULSE_MAX 1204000

#elif defined(USE_CYBOT_04)

#define PWM_PULSE_MIN 280000
#define PWM_PULSE_MAX 1225000

#elif defined(USE_CYBOT_09)

#define PWM_PULSE_MIN 259000
#define PWM_PULSE_MAX 1246000

#elif defined(USE_CYBOT_11)

#define PWM_PULSE_MIN 295750
#define PWM_PULSE_MAX 1288000

#elif defined(USE_CYBOT_17)

#define PWM_PULSE_MIN 248500
#define PWM_PULSE_MAX 1204000

#elif defined(USE_CYBOT_22)

#define PWM_PULSE_MIN 248500
#define PWM_PULSE_MAX 1267000

#elif defined(USE_CYBOT_24)

#define PWM_PULSE_MIN 253750
#define PWM_PULSE_MAX 1235500

#elif defined(USE_CYBOT_25)

#define PWM_PULSE_MIN 285250
#define PWM_PULSE_MAX 1246000


#elif defined(USE_CYBOT_26)

#define PWM_PULSE_MIN 311500
#define PWM_PULSE_MAX 1309000

#else

#define PWM_PULSE_MIN 750000	// 750μs
#define PWM_PULSE_MAX 2250000	// 2250μs

#endif

#endif