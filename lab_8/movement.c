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

static int cur_speed;


static double cur_ang_rad();
static double tar_ang_rad();
static double norm_ang(double angle);

static char str[50];

int arrived();
int is_facing();
void handle_movement();
void start_movement();
void spin();

/**
 * Initilize Movement
 * @param interface
 */
void init_movement(oi_t *interface) {
	cyBot = interface;
	oi_init(cyBot);
	oi_setWheels(0, 0);

	cur_pos.a = 0;
	cur_pos.x = 0;
	cur_pos.y = 0;

	tar_pos.a = 0;
	tar_pos.x = 0;
	tar_pos.y = 0;

	move_state = STOP;
	cur_speed = 0;
}

void movement_update() {

	// Clear data
    oi_update(cyBot);

	change.x = cyBot->distance * cosf(cur_ang_rad());		// Set change in x position
	change.y = cyBot->distance * sinf(cur_ang_rad());		// Set change in y position
	change.a = cyBot->angle;								// Set change in rotation

	cur_pos.x += change.x;		// Update global x position
	cur_pos.y += change.y;		// Update global y position
	cur_pos.a += change.a;		// Update global angle
	
	sprintf(str, "(%lf, %lf, %lf) -> (%lf, %lf, %lf) at speed %lf\r\n", cur_pos.x, cur_pos.y, cur_pos.a, tar_pos.x, tar_pos.y, tar_pos.a, cur_speed);
	uart_sendStr(str);

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

static int get_direction() {
	return 1;
	double m_x = tar_pos.x - cur_pos.x;			// Distance to move in the x direction
	double m_y = tar_pos.y - cur_pos.y;			// Distance to move in the y direction

	// If the angle is behind the cyBot, then go that way

	if (atan2(m_y, m_x) - norm_ang(cur_ang_rad()) < 0.0) {
		return -1;
	}

	return 1;
}


int arrived(){
	double m_x = tar_pos.x - cur_pos.x;			// Distance to move in the x direction
	double m_y = tar_pos.y - cur_pos.y;			// Distance to move in the y direction
	double m_d = sqrt(m_x*m_x + m_y*m_y);		// Distance to move along the hypotenuse

	if (m_d <= POS_PRECISION) {
		return 1;
	}
	return 0;
}

int is_facing(){
	return 1;
	// Project target onto current
	double a_p = atan2(tar_pos.y - cur_pos.y, tar_pos.x - cur_pos.x);

	#define MARGIN 0.1

	return norm_ang(fabs(cur_ang_rad() - a_p)) <= MARGIN;
}
void start_movement(){
	int dir = get_direction();
	oi_setWheels(dir * MIN_SPEED, dir * MIN_SPEED);
	cur_speed = MIN_SPEED;
	move_state = ACCEL;
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
	if (arrived()) {
		oi_setWheels(0, 0);
		cur_speed = 0;
		move_state = STOP;
		return;
	}

	int dir = get_direction();

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
void set_target(double x, double y, double a) {
	tar_pos.x = x;
	tar_pos.y = y;
	tar_pos.a = a;
}

/**
 * Set Target Angle
 * @param a
 */
void set_target_angle(double a) {
	tar_pos.a = a;
}

/**
 * Set Target Position
 * @param x
 * @param y
 */
void set_target_position(double x, double y) {
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
