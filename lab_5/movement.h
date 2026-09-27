#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "open_interface.h"
#include "handle_data.h"
#include "button.h"
#include "lcd.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <math.h>


typedef struct Pos {
	double a;	// Angular Heading in degrees from origin
	double x;	// X position
	double y;	// Y position
} Pos_t;



/**
 * @name Move Absolute
 * @brief Moves the CyBot to Absolute coordinates
 * @param x x-coordinate (mm)
 * @param y y-coordinate (mm)
 * @param do_avoid true/false
 */
void move_abs(oi_t *cyBot, double x, double y, int do_avoid);


/**
 * @name Turn To Absolute Angle
 * @param cyBot
 * @param target_deg Target Rotation Degrees
 */
void turn_abs(oi_t *cyBot, double target_deg);


/**
 * @name Turn to Relitive Angle
 * @param cyBot
 * @param deg Target rotation change
 */
void turn_rel(oi_t *cyBot, double deg);


/**
 * @name Move Forward
 * @param cyBot 
 * @param mm (millimeters)
 */
void move_forward(oi_t *cyBot, int mm);

/**
 * @name Move Backward
 * @param cyBot 
 * @param mm (millimeters)
 */
void move_backward(oi_t *cyBot, int mm);

/**
 * @name Move Relative 
 * @brief Moves internally without 
 * @param cyBot
 * @param mm positive for forward, negative for backward (millimeters)
 * @param do_avoid true/false
 */
void move_rel(oi_t *cyBot, int mm, int do_avoid);


/**
 * @name Turn Clockwise
 * @param cyBot
 * @param degrees
 */
void turn_cw(oi_t *cyBot, int degrees);

/**
 * @name Turn Counter-clockwise
 * @param cyBot
 * @param degrees
 */
void turn_ccw(oi_t *cyBot, int degrees);

/**
 * @name Turn Internal
 * @param cyBot
 * @param target_deg Target Rotation Degrees
 * @internal
 * @private
 * @deprecated 9/24/26
 */
void turn_internal(oi_t *cyBot, float target_deg);

/**
 * @name Has Collided on the left side
 * @param cyBot
 * @returns if the left has collided
 */
int has_collided_left(oi_t *cyBot);

/**
 * @name Has Collided on the right side
 * @param cyBot
 * @returns if the right has collided
 */
int has_collided_right(oi_t *cyBot);

/**
 * @name Has Collided on either side
 * @param cyBot
 * @returns if the has collided
 */
int has_collided(oi_t *cyBot);


/**
 * @name Avoid obstacle
 * @param cyBot
 */
void avoid(oi_t *cyBot);

/**
 * @name Get Position
 * @brief Get Position of the CyBot
 * @returns X,Y coordinates along with the heading in a struct
 */
Pos_t getPosition();


void rot_calibrate(oi_t *cyBot);


#endif
