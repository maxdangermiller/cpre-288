#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "open_interface.h"
#include <math.h>


// Struct to store a position
typedef struct {
	double a;	// Angular Heading 	(degrees)
	double x;	// X position		(mm)
	double y;	// Y position		(mm)
} Pos_t;


// Enum to store what state the movement is in
typedef enum MoveState {
	MS_STOP = 0, 		
	MS_ACCEL = 1, 
	MS_CRUISE = 2, 
	MS_DECEL = 3, 
	MS_CREEP = 4,
	MS_SPIN = 5
} STATE;

typedef enum AvoidState {
	AS_NONE,
	AS_AVOID_LEFT,
	AS_AVOID_RIGHT
} AvoidState_t;

typedef enum AvoidPhase {
	AP_BACK, 		
	AP_TURN1, 
	AP_SIDE_MOVE, 
	AP_TURN2, 
	AP_FORWARD
} AvoidPhase_t;


void move_init(oi_t *interface);

void move_update();

void move_set_target(double x, double y, double a);
void move_set_target_angle(double a);
void move_set_target_position(double x, double y);

void move_forward(double mm);
void move_backward(double mm);
void move_turn_cw(double degrees);
void move_turn_ccw(double degrees);
void move_turn_rel(double degrees);
void move_abort();
void move_resume();

Pos_t move_get_target_pos();
Pos_t move_get_current_pos();
STATE move_get_state();


#endif
