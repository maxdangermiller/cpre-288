#include "movement.h"

#define POS_PRECISION 0.5

#define POS_ACCEL 20
#define MIN_SPEED 20
#define MAX_SPEED 150


static struct Pos {
	double a;	// Angular Heading in degrees from origin
	double x;	// X position
	double y;	// Y position
} cur_pos, tar_pos, change;

static enum STATE {
	STOP, ACCEL, CRUISE, DECEL, CREEP
} move_state;

static oi_t *cyBot;

static int cur_speed = 0;


static double cur_ang_rad();
static double tar_ang_rad();
static double norm_ang(double angle);

void calc_traj();
int arrived();
int is_facing();
void handle_movement();
void start_movement();
void spin();

/**
 * Initilize Movement
 * @param interface
 */
void move_init(oi_t *interface) {
	cyBot = interface;
	oi_init(cyBot);
	oi_setWheels(0, 0);

	cur_pos.a = 0;
	cur_pos.x = 0;
	cur_pos.y = 0;
	tar_pos.a = 0;
	tar_pos.x = 0;
	tar_pos.y = 0;
}

void update() {

	// Clear data
    oi_update(cyBot);

	change.x = cyBot->distance * cosf(cur_ang_rad());		// Set change in x position
	change.y = cyBot->distance * sinf(cur_ang_rad());		// Set change in y position
	change.a = cyBot->angle;								// Set change in rotation

	cur_pos.x += change.x;		// Update global x position
	cur_pos.y += change.y;		// Update global y position
	cur_pos.a += change.a;		// Update global angle
	
	/**
	 * Steps:
	 * Find current trjectory: is it moving?
	 * 		If yes: Are we actually going the right way????
	 *          If yes: keep going
	 *              Check if we need to keep accelerating, or if we need to start slowing down
	 *          If no: stop. We'll fix it on the next one bois
	 * 		If no: are we facing the point
	 * 			If yes: Start movement
	 * 			If no: Start/Continue spin
	 */
	if(cur_speed != 0){ //moving
		if(is_facing()) {
			// Move daddy
			handle_movement();
		}
	}
	else if(!arrived()){
		if (is_facing()){
			start_movement();
		}
		else {
			spin();
		}
	}
	
}


int arrived(){

}
int is_facing(){
	// Project target onto current
	double a_p = atan2(tar_pos.y - cur_pos.y, tar_pos.x - cur_pos.x);

	#define MARGIN 0.1

	return norm_ang(fabs(cur_ang_rad() - a_p)) <= MARGIN;
}
void start_movement(){

}

static int getting_close() {
	int xdir = fabs(tar_pos.x - (cur_pos.x + 5 * change.x)) <= POS_PRECISION;
	int ydir = fabs(tar_pos.y - (cur_pos.y + 5 * change.y)) <= POS_PRECISION;

	return xdir && ydir;
}

void handle_movement(){
	
	
	double m_x = tar_pos.x - cur_pos.x;			// Distance to move in the x direction
	double m_y = tar_pos.y - cur_pos.y;			// Distance to move in the y direction
	double m_d = sqrt(m_x*m_x + m_y*m_y);		// Distance to move along the hypotenuse

	// if there, stop
	if (fabs(m_x - m_y) <= POS_PRECISION) {
		oi_setWheels(0, 0);
		move_state = STOP;
		return;
	}

	int dir = 1;

	// If the angle is behind the cyBot, then go that way
	if (atan2(m_y, m_x) - norm_ang(cur_ang_rad()) < 0.0) {
		dir = -1;
	}
	if (move_state != DECEL && getting_close()) {
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
	}

	oi_setWheels(dir * cur_speed, dir * cur_speed);

}
void spin(){

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
 * @param x
 * @param y
 */
void move_set_target_position(double x, double y) {
	tar_pos.x = x;
	tar_pos.y = y;
}

/**
 * Move forward
 * @param mm
 */
void move_forward(double mm) {
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
void turn_cw(int degrees) {
	tar_pos.a -= degrees;
}

/**
 * Turn Counter-clockwise
 * @param degrees
 */
void turn_ccw(int degrees) {
	tar_pos.a += degrees;
}

/**
 * Current angle in radians
 */
static double cur_ang_rad() {
	return cur_pos.a * (M_PI / 180.0);
}

/**
 * Current angle in radians
 */
static double tar_ang_rad() {
	return tar_pos.a * (M_PI / 180.0);
}

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
