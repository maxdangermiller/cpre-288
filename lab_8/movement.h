#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "open_interface.h"
#include <math.h>



void init_movement(oi_t *interface);

void movement_update();

void set_target(double x, double y, double a);
void set_target_angle(double a);
void set_target_position(double x, double y);

void move_forward(int mm);
void move_backward(int mm);
void turn_cw(int degrees);
void turn_ccw(int degrees);


#endif
