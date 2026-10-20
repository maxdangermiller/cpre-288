/**
 * @file movement.c
 * @authors Max Miller, Miles Davis
 * @brief This file manages all movement for the cyBot
 * @version 0.4
 * @date 2026-10-20
 * 
 */
#include "movement.h"
#include "uart.h"
#include <stdio.h>

// CyBot 24
#define DIS_CALIBRATION 1.0		// Calibration macro for the distance
#define ANG_CALIBRATION 1.055	// Calibration macro for rotation
#define ANG_CALIBRATION 1.0		// Calibration macro for rotation

#define POS_PRECISION 2.0		// mm, how close counts as "there"
#define ANG_PRECISION 1.0		// degrees, how close a spin needs to get before stopping
#define FINE_DIST 20.0			// mm, inside this we stop steering and only care about distance along our heading
#define FACE_MARGIN 3.0			// degrees, max heading error allowed to start moving
#define DRIVE_MARGIN 10.0		// degrees, max heading error while moving before we stop and re-spin
#define LOOKAHEAD 8				// number of updates ahead to look when deciding to slow down

#define MAX_SPEED 150			// Maximum speed the cyBot is allowed to travel when moving 	(mm/s)
#define MIN_SPEED 20			// Minimum speed the cyBot is allowed to travel when moving 	(mm/s)
#define POS_ACCEL 20			// Acceleration for moving forward/backward 					(mm/s)
#define Kp 2.0					// Steering correction (mm/s per degree of heading error)		(mm/(s*degree))

#define MAX_ROT_SPEED 100		// Maximum speed the cyBot is allowed to travel when rotating 	(mm/s)
#define MIN_ROT_SPEED 20		// Minimum speed the cyBot is allowed to travel when rotating 	(mm/s)
#define ROT_ACCEL 10			// Acceleration for rotation 									(mm/s)
#define ROT_Kp 2.0				// Spin speed (mm/s per degree of heading error)				(mm/(s*degree))

#define DEBUG_MOVEMENT			// Debug enable macro


Pos_t cur_pos, tar_pos, change;

static STATE move_state;

static oi_t *cyBot;				// Store cyBot reference

static int cur_speed;			// Current wheel speed (mm/s), always positive
static int move_dir;			// Direction of the current move: 1 forward, -1 backward
static double dist_change;		// Distance (mm) traveled since the last update

static char str[100];			// For debugging 


// Internal methods
static double cur_ang_rad();
static double tar_ang_rad();
static double norm_ang(double angle);
static double dist_left();
static double dist_along();
static double dist_to_go();
static double face_angle();
static int get_direction();
static int is_moving();
static int arrived();
static int is_facing();
static int is_at_angle();
static int getting_close();
static void start_movement();
static void handle_movement();
static void spin(double target);
static void halt();


/**
 * Initialize Movement
 * Expects oi_init to already have been called on the interface
 * @param interface
 */
void move_init(oi_t *interface) {
	cyBot = interface;
	oi_setWheels(0, 0);
	oi_update(cyBot);	// Clear data
	oi_setMotorCalibration(1.0, 1.0);

	cur_pos.a = 0;
	cur_pos.x = 0;
	cur_pos.y = 0;

	tar_pos.a = 0;
	tar_pos.x = 0;
	tar_pos.y = 0;

	move_state = STOP;
	cur_speed = 0;
	move_dir = 1;
	dist_change = 0;
}


/**
 * Movement Update
 * @brief Main update method for all movement of the cyBot. Call AS MUCH AS POSSIBLE
 */
void move_update() {

    oi_update(cyBot);

	change.a = cyBot->angle * ANG_CALIBRATION;			// Set change in rotation
	cur_pos.a += change.a;								// Update global angle

	dist_change = cyBot->distance * DIS_CALIBRATION;	// Set distance traveled
	change.x = dist_change * cos(cur_ang_rad());		// Set change in x position
	change.y = dist_change * sin(cur_ang_rad());		// Set change in y position

	cur_pos.x += change.x;		// Update global x position
	cur_pos.y += change.y;		// Update global y position

#ifdef DEBUG_MOVEMENT
	snprintf(str, sizeof(str), "(%.2lf, %.2lf, %.2lf) -> (%.2lf, %.2lf, %.2lf) state %d speed %d\r\n",
			cur_pos.x, cur_pos.y, cur_pos.a, tar_pos.x, tar_pos.y, tar_pos.a, move_state, cur_speed);
	uart_sendStr(str);
#endif

	/**
	 * Steps:
	 * Are we at the target position?
	 * 		If no: are we facing the point (or close enough that it doesn't matter)?
	 * 			If yes: start moving, or keep moving if we already are
	 * 			If no: if we're driving, stop. We'll fix it on the next one bois
	 * 			       otherwise start/continue spin to face the point
	 * 		If yes: are we at the target angle?
	 * 			If no: stop if driving, then start/continue spin to the target angle
	 * 			If yes: stop
	 */
	if (!arrived()) {
		if (is_facing()) {
			if (is_moving()) {
				handle_movement();
			}
			else {
				start_movement();
			}
		}
		else if (is_moving()) {
			halt();
		}
		else {
			spin(face_angle());
		}
	}
	else if (!is_at_angle()) {
		if (is_moving()) {
			halt();
		}
		else {
			spin(tar_pos.a);
		}
	}
	else {
		halt();
	}

}


/**
 * Straight line distance (mm) to the target
 */
static double dist_left() {
	double m_x = tar_pos.x - cur_pos.x;			// Distance to move in the x direction
	double m_y = tar_pos.y - cur_pos.y;			// Distance to move in the y direction
	return sqrt(m_x*m_x + m_y*m_y);				// Distance to move along the hypotenuse
}


/**
 * Distance (mm) to the target projected onto our current heading.
 * Positive if the target is ahead, negative if it is behind
 */
static double dist_along() {
	double m_x = tar_pos.x - cur_pos.x;
	double m_y = tar_pos.y - cur_pos.y;
	return m_x * cos(cur_ang_rad()) + m_y * sin(cur_ang_rad());
}


/**
 * Distance (mm) we still need to drive.
 * When we're close, the angle to the target gets noisy, so only count the distance along our heading
 */
static double dist_to_go() {
	if (dist_left() <= FINE_DIST) {
		return fabs(dist_along());
	}
	return dist_left();
}


/**
 * Angle (degrees) the CyBot needs to face to drive to the target, taking direction into account
 */
static double face_angle() {
	double a = atan2(tar_pos.y - cur_pos.y, tar_pos.x - cur_pos.x) * (180.0 / M_PI);

	// If we're backing up, the back of the cyBot faces the target
	if (get_direction() < 0) {
		a += 180.0;
	}

	return a;
}


/**
 * Which way to drive to get to the target
 * @returns 1 for forward, -1 for backward
 */
static int get_direction() {
	// Close to the target, just go whichever way along our heading gets us there
	if (dist_left() <= FINE_DIST) {
		return dist_along() < 0.0 ? -1 : 1;
	}

	// If the target is behind the cyBot (more than 90 degrees off), then back up to it
	double a = atan2(tar_pos.y - cur_pos.y, tar_pos.x - cur_pos.x) * (180.0 / M_PI);

	if (fabs(norm_ang(a - cur_pos.a)) > 90.0) {
		return -1;
	}

	return 1;
}


/**
 * Are we currently driving (not stopped or spinning)
 */
static int is_moving() {
	return move_state != STOP && move_state != SPIN;
}


/**
 * Are we at the target position, or going to be by the next update
 */
static int arrived() {
	return dist_to_go() - fabs(dist_change) <= POS_PRECISION;
}


/**
 * Are we facing the target position (or the back of the cyBot is if backing up)
 */
static int is_facing() {
	// Close enough that we don't steer anymore, so we're always "facing"
	if (dist_left() <= FINE_DIST) {
		return 1;
	}

	// Allow more error once we're already moving so we don't keep stopping to re-spin
	double margin = is_moving() ? DRIVE_MARGIN : FACE_MARGIN;

	return fabs(norm_ang(face_angle() - cur_pos.a)) <= margin;
}


/**
 * Are we at the target angle
 */
static int is_at_angle() {
	return fabs(norm_ang(tar_pos.a - cur_pos.a)) <= FACE_MARGIN;
}


/**
 * Are we going to be there within the next LOOKAHEAD updates
 */
static int getting_close() {
	return dist_to_go() - LOOKAHEAD * fabs(dist_change) <= POS_PRECISION;
}


/**
 * Start Movement
 * 
 */
static void start_movement() {
	move_dir = get_direction();
	cur_speed = MIN_SPEED;
	move_state = ACCEL;
	oi_setWheels(move_dir * cur_speed, move_dir * cur_speed);
}


/**
 * Handle Movement
 * 
 */
static void handle_movement() {

	int dir = get_direction();

	// If we overshot and need to go the other way, stop. We'll start back up next update
	if (dir != move_dir) {
		halt();
		return;
	}

	if ((move_state == ACCEL || move_state == CRUISE) && getting_close()) {
		move_state = DECEL;
	}

	// Handle Acceleration
	switch (move_state) {
		case ACCEL:
			if (cur_speed < MAX_SPEED) {
				cur_speed += POS_ACCEL;
			}

			if (cur_speed >= MAX_SPEED) {
				cur_speed = MAX_SPEED;
				move_state = CRUISE;
			}
			break;

		case DECEL:
			if (cur_speed > MIN_SPEED) {
				cur_speed -= POS_ACCEL;
			}

			if (cur_speed <= MIN_SPEED) {
				cur_speed = MIN_SPEED;
				move_state = CREEP;
			}
			break;

		default:
			// CRUISE and CREEP hold their speed
			break;
	}

	// Steer back towards the target. Skip it when close since the angle to the target gets noisy
	double error = 0.0;
	if (dist_left() > FINE_DIST) {
		error = norm_ang(face_angle() - cur_pos.a) * Kp;
	}

	// oi_setWheels takes (right, left). Positive error means turn ccw, so speed up the right wheel
	oi_setWheels((int)(dir * cur_speed + error), (int)(dir * cur_speed - error));

}


/**
 * Spin in place towards an angle
 * @param target angle in degrees
 */
static void spin(double target) {

	double error = norm_ang(target - cur_pos.a);

	// If we are going to be there by the next update, go ahead and stop
	if (fabs(error) - fabs(change.a) <= ANG_PRECISION) {
		halt();
		return;
	}

	// Positive error is counter-clockwise
	int dir = error > 0 ? 1 : -1;

	// Slow down as we get close to the angle
	int tar_speed = (int)(fabs(error) * ROT_Kp);
	if (tar_speed < MIN_ROT_SPEED) {
		tar_speed = MIN_ROT_SPEED;
	}
	if (tar_speed > MAX_ROT_SPEED) {
		tar_speed = MAX_ROT_SPEED;
	}

	if (move_state != SPIN) {
		move_state = SPIN;
		cur_speed = MIN_ROT_SPEED;
	}
	else if (cur_speed + ROT_ACCEL < tar_speed) {
		cur_speed += ROT_ACCEL;
	}
	else {
		cur_speed = tar_speed;
	}

	// oi_setWheels takes (right, left). Counter-clockwise means right wheel forward
	oi_setWheels(dir * cur_speed, -dir * cur_speed);

}


/**
 * Stop the wheels
 */
static void halt() {
	if (move_state != STOP) {
		oi_setWheels(0, 0);
	}
	cur_speed = 0;
	move_state = STOP;
}


/**
 * Set Target Position and Angle
 * @param x
 * @param y
 * @param a
 */
void move_set_target(double x, double y, double a) {
	tar_pos.x = x;
	tar_pos.y = y;
	tar_pos.a = a;
}


/**
 * Set Target Angle
 * @param a
 */
void move_set_target_angle(double a) {
	tar_pos.a = a;
}


/**
 * Set Target Position
 * The target angle is set to the direction of travel so we don't spin back around once we get there
 * @param x
 * @param y
 */
void move_set_target_position(double x, double y) {
	if (x != tar_pos.x || y != tar_pos.y) {
		tar_pos.a = atan2(y - tar_pos.y, x - tar_pos.x) * (180.0 / M_PI);
	}
	tar_pos.x = x;
	tar_pos.y = y;
}


/**
 * Move forward
 * @param mm
 */
void move_forward(int mm) {
	tar_pos.x += mm * cos(tar_ang_rad());
	tar_pos.y += mm * sin(tar_ang_rad());
}


/**
 * Move Backward
 * @param mm
 */
void move_backward(int mm) {
	tar_pos.x -= mm * cos(tar_ang_rad());
	tar_pos.y -= mm * sin(tar_ang_rad());
}


/**
 * Turn clockwise
 * @param degrees
 */
void move_turn_cw(int degrees) {
	tar_pos.a -= degrees;
}


/**
 * Turn Counter-clockwise
 * @param degrees
 */
void move_turn_ccw(int degrees) {
	tar_pos.a += degrees;
}


/**
 * Current angle in radians
 */
static double cur_ang_rad() {
	return cur_pos.a * (M_PI / 180.0);
}


/**
 * Target angle in radians
 */
static double tar_ang_rad() {
	return tar_pos.a * (M_PI / 180.0);
}


/**
 * Normalize an angle in degrees to [-180, 180)
 */
static double norm_ang(double angle) {
    angle = fmod(angle, 360.0);

    // Force it into the range [0, 360) if it's negative
    if (angle < 0) {
        angle += 360.0;
    }

    // Shift it down to the range [-180, 180)
    if (angle >= 180.0) {
        angle -= 360.0;
    }

    return angle;
}

/**
 * @brief Get the target pos object
 * 
 * @return Pos_t  tar_pos
 */
Pos_t move_get_target_pos() {
	return tar_pos;
}


/**
 * @brief Get the current pos object
 * 
 * @return Pos_t cur_pos
 */
Pos_t move_get_current_pos() {
	return cur_pos;
}


/**
 * @brief Get Move State
 * 
 * @return STATE 
 */
STATE move_get_state() {
	return move_state;
}
