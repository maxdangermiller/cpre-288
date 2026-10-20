#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "open_interface.h"
#include <math.h>


typedef struct {
	double a;	// Angular Heading 	(degrees)
	double x;	// X position		(mm)
	double y;	// Y position		(mm)
} Pos_t;

typedef enum {
	STOP = 0, 
	ACCEL = 1, 
	CRUISE = 2, 
	DECEL = 3, 
	CREEP = 4,
	SPIN = 5
} STATE;


void move_init(oi_t *interface);

void move_update();

void move_set_target(double x, double y, double a);
void move_set_target_angle(double a);
void move_set_target_position(double x, double y);

void move_forward(int mm);
void move_backward(int mm);
void move_turn_cw(int degrees);
void move_turn_ccw(int degrees);

Pos_t move_get_target_pos();
Pos_t move_get_current_pos();
STATE move_get_state();


#endif
