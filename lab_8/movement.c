/**
 * @name Movement
 * @headerfile movement.h
 * @version 2.0
 * @author Max Miller
 */

#include "movement.h"

// #define DIS_CALIBRATION 1.02
// #define ANG_CALIBRATION 1.055

// CyBot 24
#define DIS_CALIBRATION 1.0
#define ANG_CALIBRATION 1.00425832

#define FAST_SPEED 150
#define FINE_SPEED 20
#define FINE_THRESHOLD_DIS 50       // mm
#define FINE_THRESHOLD_ANG 25       // degrees
#define PRECISION 0.5
#define Kp 0.04

#define POS_ACCEL 20


// Store current position of the CyBot
static double pos_x = 0.0;          // mm
static double pos_y = 0.0;          // mm
static double pos_deg = 0.0;        // degrees

static char str[80];


/**
 * @name Move Absolute
 * @brief Moves the CyBot to Absolute coordinates
 * @param x x-coordinate (mm)
 * @param y y-coordinate (mm)
 * @param do_avoid true/false
 */
void move_abs(oi_t *cyBot, double x, double y, int do_avoid) {
	// Needs to calculate an angle to turn to so that we can actually go to that position
	// Then turn to that angle
	// Then go the correct distance to get there

    sprintf(str, "\r\nCurrent position: (%.2lf, %.2lf); target: (%.2lf, %.2lf)\r\n", pos_x, pos_y, x, y);
    cyBot_sendString(str);
	
	double rad = atan2(y - pos_y, x - pos_x);   // Angle to get there in radians
	double deg = rad * 180.0 / M_PI;	        // Angle to get there in degrees

	sprintf(str, "Going to angle %.2lf rad or %.2lf deg\r\n", rad, deg);
	cyBot_sendString(str);

	double m_x = fabs(pos_x - x);			// Distance to move in the x direction
	double m_y = fabs(pos_y - y);			// Distance to move in the y direction
	double m_d = sqrt(m_x*m_x + m_y*m_y);	// Distance to move along the hypotenuse

	double dis_change;						// Var to store the change in each iteration
	double deg_change;						// Var to store the change in each iteration
	double change_x;						// Stores the change in the x direction in each loop
	double change_y;						// Stores the change in the y direction in each loop

    double cur_speed = 30;

	double error;

	int slow = 0;

	sprintf(str, "\r\nCurrent position: (%.2lf, %.2lf); target: (%.2lf, %.2lf)\r\n", pos_x, pos_y, x, y);
	cyBot_sendString(str);



	// If the CyBot needs to rotate to go to that position, go ahead and rotate it
	if (fabs(pos_deg - deg) > PRECISION) {
	    cyBot_sendString("\r\nTurning!\r\n\r\n");
		turn_abs(cyBot, deg);
		deg = pos_deg;
	}

	sprintf(str, "Going to angle %.2lf=%.2lf rad or %.2lf deg\r\n", rad, pos_deg / M_PI * 180, pos_deg);
	cyBot_sendString(str);

	// Set speed to FAST_SPEED
	oi_setWheels(cur_speed, cur_speed);

	sprintf(str, "Distance: %.2lf\r\n", m_d);
	cyBot_sendString(str);

	// While the distance left is greater than PRECISION, keep iterating
	while (!isnan(m_d) && m_d > PRECISION) {
        if (cur_speed < FAST_SPEED) {
            cur_speed += POS_ACCEL;
        }
        else {
            cur_speed = FAST_SPEED;
        }
		
		oi_update(cyBot);									// Update the CyBot Sensors
		
		dis_change = cyBot->distance * DIS_CALIBRATION;		// Set the distance (mm) since last update
		deg_change = cyBot->angle * ANG_CALIBRATION;		// Set the rotation (deg) since last update

		// Dynamic Proportional Correction
		error = (deg_change - atan2(y - pos_y, x - pos_x) * (180.0 / M_PI)) * Kp;

		if (do_avoid && has_collided(cyBot)) {				// if do_avoid is enabled check if we have collided
			avoid(cyBot);										// Run Avoid Function
			continue;											// Start the movement loop over again
		}

		if (!slow) {
			oi_setWheels((int)(cur_speed - error), (int)(cur_speed + error)); // Reset Movement
		}
		else {
			oi_setWheels((int)(FINE_SPEED - error), (int)(FINE_SPEED + error)); // Reset Movement

		}



		// rad = atan2(y - pos_y, x - pos_x);
		deg += deg_change;
		pos_deg = deg;
		rad = deg * (M_PI / 180.0);

		change_x = dis_change * cosf(rad);					// Find change in the x direction
		change_y = dis_change * sinf(rad); 					// Find change in the y direction


		pos_x += change_x;									// Update current actual position in the x direction
		pos_y += change_y;									// Update current actual position in the x direction

		m_x = fabs(pos_x - x);								// Update the distance still to travel in the x direction
		m_y = fabs(pos_y - y);								// Update the distance still to travel in the y direction
		m_d = sqrt(m_x*m_x + m_y*m_y);						// Update the distance still to travel

		// sprintf(str, "Distance (update): %.2lf, degrees: %.2lf\r\n", m_d, deg);
		// sprintf(str, "cyBot->leftEncoderCount: %.2lf, cyBot->rightEncoderCount: %.2lf ==> Error: %.8lf\r\n", m_d, deg, error);
		sprintf(str, "(%.2lf, %.2lf) ==> (%.2lf, %.2lf)\r\n", pos_x, pos_y, x, y);
		cyBot_sendString(str);

		if (!slow && !isnan(m_d) && m_d - 5 * dis_change <= PRECISION) {
		    cyBot_sendString("Slowing down\r\n");
		    slow = true;
        }

		// If we are going to be there by the next time we do a loop, go ahead and stop
		if (!isnan(m_d) && m_d - dis_change <= PRECISION) {
			break;
		}
	}

	oi_setWheels(0, 0); // stop

	oi_update(cyBot);
	dis_change = cyBot->distance * DIS_CALIBRATION;
    pos_deg += (cyBot->angle * ANG_CALIBRATION);
    rad = pos_deg * (M_PI / 180.0);
	
	pos_x += dis_change * cosf(rad);
	pos_y += dis_change * sinf(rad);

}


/**
 * @name Turn To Absolute Angle
 * @param cyBot
 * @param target_deg Target Rotation Degrees
 */
void turn_abs(oi_t *cyBot, double target_deg) {
	// turn_internal(cyBot, target_deg - pos_deg);
	
    // If the angle is negative, the direction is negative (aka clock-wise),
    // otherwise the direction is positive (aka counter-clock-wise)
	int dir = (pos_deg - target_deg) < 0 ? -1 : 1;

	int slow = 0;

    oi_update(cyBot); // Clear preivious update

	oi_setWheels(-dir * FAST_SPEED, dir * FAST_SPEED);


	// sprintf(str, "Turning (abs): from %.2lf° to %.2lf°!", pos_deg, target_deg);
	// cyBot_sendString(str);

	while (dir * (pos_deg - target_deg) > PRECISION) {
		oi_update(cyBot);

		// Set the change in the angle since the last update
		double ang_change = (cyBot->angle * ANG_CALIBRATION);
		
		pos_deg += ang_change;

	    // sprintf(str, "Direction: %d, target: %.2lf, current: %.2lf, delta: %.2lf, change: %.2lf", dir, target_deg, pos_deg, pos_deg - target_deg, ang_change);
	    // cyBot_sendString(str);

	    // sprintf(str, ", prediction: %0.2lf\r\n", pos_deg - target_deg + 3 * ang_change);
	    // cyBot_sendString(str);

        // sprintf(str, "Turning (abs): Final angle %.2lf°/%.2lf°! Delta: %.2lf=%.2lf\r\n", pos_deg, target_deg, cyBot->angle, ang_change);
        // cyBot_sendString(str);

	    // If we are going to be there by there in the next three loops, slow down
        if (!slow && dir * (pos_deg - target_deg + 3 * ang_change) <= PRECISION) {
            // cyBot_sendString("Slowing down\r\n");
            oi_setWheels(-dir * FINE_SPEED, dir * FINE_SPEED);
            slow = true;
        }

		// If we are going to be there by the next time we do a loop, go ahead and stop
		if (dir *(pos_deg - target_deg + ang_change) <= PRECISION) {
			break;
		}
	}

	// Stop
	oi_setWheels(0, 0);

	oi_update(cyBot);
	pos_deg += (cyBot->angle * ANG_CALIBRATION);

	sprintf(str, "Turning (abs): Final angle %.2lf° where the target was %.2lf°!\r\n", pos_deg, target_deg);
	cyBot_sendString(str);
}


/**
 * @name Turn to Relitive Angle
 * @param cyBot
 * @param deg Target rotation change
 */
void turn_rel(oi_t *cyBot, double deg) {
    turn_abs(cyBot, pos_deg + deg);
}


/**
 * @name Move Forward
 * @param cyBot 
 * @param mm (millimeters)
 */
void move_forward(oi_t *cyBot, int mm) {
	move_rel(cyBot, mm, true);
	// double rad = pos_deg * (M_PI / 180.0);
	// move_abs(cyBot, pos_x + (double)mm * cos(rad), pos_y + (double)mm * sin(rad), true);
}


/**
 * @name Move Backward
 * @param cyBot 
 * @param mm (millimeters)
 */
void move_backward(oi_t *cyBot, int mm) {
	move_rel(cyBot, -mm, true);
	// double rad = pos_deg * (M_PI / 180.0);
	// move_abs(cyBot, pos_x - (double)mm * cos(rad), pos_y - (double)mm * sin(rad), true);
}


/**
 * @name Move Relative 
 * @brief Moves internally without 
 * @param cyBot
 * @param mm positive for forward, negative for backward (millimeters)
 * @param do_avoid true/false
 */
void move_rel(oi_t *cyBot, int mm, int do_avoid) {
	int dir = mm < 0 ? -1 : 1;

	double rad = pos_deg / 360.0 * 2 * M_PI;

	double dis_change = 0.0;
	double dis_tot = 0.0;

    double cur_speed = 30;

    // Clear data
    oi_update(cyBot);

	oi_setWheels(dir * cur_speed, dir * cur_speed);

	sprintf(str, "Distance: %d.00\r\n", mm);
	cyBot_sendString(str);

	int slow = 0;

	// dir * (mm - dis_tot) > PRECISION
	while (fabs(dir * mm - dir * dis_tot) > PRECISION) {
        if (!slow && cur_speed < FAST_SPEED) {
            cur_speed += POS_ACCEL;
        }
        else if (!slow) {
            cur_speed = FAST_SPEED;
        }
        else if (slow && cur_speed >= FINE_SPEED) {
            cur_speed -= POS_ACCEL;
        }
        else {
            cur_speed = FINE_SPEED;
        }

		oi_update(cyBot);									// Update the CyBot Sensors


		dis_change = cyBot->distance * DIS_CALIBRATION;	    // Update distance traveled
		dis_tot += dis_change;                              // Update total distance traveled

		if (do_avoid && has_collided(cyBot)) {				// if do_avoid is enabled check if we have collided
			avoid(cyBot);										// Run Avoid Function
			continue;											// Start the movement loop over again
		}

		oi_setWheels(dir * cur_speed, dir * cur_speed);

		// sprintf(str, "Move Rel (update): distance: %.2lf, to go: %.2lf\r\n", dis_tot, dir * (mm - dis_tot));
		// cyBot_sendString(str);

		if (dir * (mm - dis_tot) < 0.0) {
			dir = -dir;
			oi_setWheels(dir * cur_speed, dir * cur_speed);
		}

		// If we are going to be there by there in the next three loops, slow down
        if (!slow && dir * (mm - dis_tot - 5 * dis_change) <= PRECISION) {
            cyBot_sendString("Slowing down\r\n");
            slow = true;
        }

		if (dir * (mm - dis_tot - dis_change) <= PRECISION) {
            break;
        }

	}

	oi_setWheels(0, 0); 								// stop

	oi_update(cyBot);									// Update one last time so we get our actual ending position

	dis_change = cyBot->distance * DIS_CALIBRATION;	    // Update distance traveled
	dis_tot += dis_change;
	pos_x += dis_tot * cosf(rad);		// Update global x position
	pos_y += dis_tot * sinf(rad);		// Update global y position

	sprintf(str, "Final Distance: %.2lf\r\n", dis_tot);
	cyBot_sendString(str);
}


/**
 * @name Turn Clockwise
 * @param cyBot
 * @param degrees
 */
void turn_cw(oi_t *cyBot, int degrees) {
	// turn_internal(cyBot, -degrees);
	turn_abs(cyBot, pos_deg - degrees);
}


/**
 * @name Turn Counter-clockwise
 * @param cyBot
 * @param degrees
 */
void turn_ccw(oi_t *cyBot, int degrees) {
	// turn_internal(cyBot, degrees);
	turn_abs(cyBot, pos_deg + degrees);
}


/**
 * @name Turn Internal
 * @param cyBot
 * @param target_deg Target Rotation Degrees
 * @internal
 * @private
 * @deprecated 9/24/26
 */
void turn_internal(oi_t *cyBot, float target_deg) {
	// Store current degrees - THIS IS THE *TRUE* VALUE AFTER OFFSET AND DIRECTION
	double cur_deg = 0.0;
	
	// If the angle is negative, the direction is negative (aka clock-wise), 
	// otherwise the direction is positive (aka counter-clock-wise)
	int direction = target_deg < 0 ? -1 : 1;

	// Start wheels in oppsite directions, after accounting for direction
	oi_setWheels(direction * FAST_SPEED, -direction * FAST_SPEED);
	
	// Run at the fast speed until the angle is within FINE_THRESHOLD_ANG
	while (direction * (target_deg - cur_deg) > FINE_THRESHOLD_ANG) {
		oi_update(cyBot);
		cur_deg += (cyBot->angle * ANG_CALIBRATION);
		// printf("Fast: %f\n", cur_deg);
	}

	// Set wheel speed to the fine speed
	oi_setWheels(direction * FINE_SPEED, -direction * FINE_SPEED);

	// Run at fine speed until the angle is within precision
	while (direction * (target_deg - cur_deg) > PRECISION) {

		// Update
		oi_update(cyBot);

		// Update current angle with the new cyBot data, taking direction and offset into account
		cur_deg += (cyBot->angle * ANG_CALIBRATION);
		// printf("Slow: %f\n", cur_deg);
	}

	oi_setWheels(0, 0); // stop

	oi_update(cyBot);
	cur_deg += (cyBot->angle * ANG_CALIBRATION);
	// printf("Final: %f = %f\n", cur_deg, cur_deg / (double)target_deg);

	pos_deg += cur_deg * direction;
}


/**
 * @name Has Collided on the left side
 * @param cyBot
 * @returns if the left has collided
 */
int has_collided_left(oi_t *cyBot) {
	return cyBot->bumpLeft;
}


/**
 * @name Has Collided on the right side
 * @param cyBot
 * @returns if the right has collided
 */
int has_collided_right(oi_t *cyBot) {
	return cyBot->bumpRight;
}


/**
 * @name Has Collided on either side
 * @param cyBot
 * @returns if the has collided
 */
int has_collided(oi_t *cyBot) {
	return has_collided_left(cyBot) || has_collided_right(cyBot);
}


/**
 * @name Avoid obstacle
 * @param cyBot
 */
void avoid(oi_t *cyBot) {
	/* 
		If collided back up 15 cm, turn 90 degrees, move laterally 25cm, then turn 90 degrees forward.
		If the collision occurs with the right bumper, the robot should initially spin 90 degrees to the left. 
		If the collision occurs with the left bumper, the platform should spin 90 degrees to the right.
		If both cyBots report a collision, then pick a direction and perform a 90 degree spin.
	*/

	// If right is triggered or if both are triggered
	if (has_collided_right(cyBot)) {
		move_rel(cyBot, -250, false);
		turn_ccw(cyBot, 90);
		move_rel(cyBot, 250, false);
		turn_cw(cyBot, 90);
		move_rel(cyBot, 250, false);
	}

	// If left is triggered
	else if (has_collided_left(cyBot)) {
		move_rel(cyBot, -250, false);
		turn_cw(cyBot, 90);
		move_rel(cyBot, 250, false);
		turn_ccw(cyBot, 90);
		move_rel(cyBot, 250, false);
	}

	// Otherwise nothing was triggered so do nothing
}


/**
 * @name Get Position
 * @brief Get Position of the CyBot
 * @returns X,Y coordinates along with the heading in a struct
 */
Pos_t getPosition() {
	Pos_t pos;

	pos.x = pos_x;
	pos.y = pos_y;
	pos.a = pos_deg;
	return pos;
}


void rot_calibrate(oi_t *cyBot) {
	lcd_init();
	button_init();
	timer_init();

	double ang = 0.0;
	double dist = 0.0;
	int btn = 0;
	int loop = true;

	while (loop) {
		btn = button_getButton();

		switch (btn) {
			case 0:
				oi_setWheels(0, 0);
				break;
			case 1:
				oi_setWheels(FINE_SPEED, -FINE_SPEED);
				break;
			case 2:
				oi_setWheels(-FINE_SPEED, FINE_SPEED);
				break;
			case 3:
				oi_setWheels(-FAST_SPEED, FAST_SPEED);
				break;
			case 4:
				loop = false;
				break;
			
			default:
				break;
		}

		oi_update(cyBot);
		ang += cyBot->angle;

		lcd_printf("Cur Deg: %04.02lf\nRotations: %02.02lf/4.00", ang, ang / 360.0);
		

	}

	lcd_printf("Offset: %10.08lf", (360.0 * 4) / ang);

	while (button_getButton() != 0) {
		timer_waitMillis(10);
	}

	while (button_getButton() != 4) {
		timer_waitMillis(10);
	}

	while (button_getButton() != 0) {
		timer_waitMillis(10);
	}

	loop = true;
	ang = 0;

	while (loop) {
		btn = button_getButton();

		switch (btn) {
			case 0:
				oi_setWheels(0, 0);
				break;
			case 1:
				oi_setWheels(-FINE_SPEED, -FINE_SPEED);
				break;
			case 2:
				oi_setWheels(FINE_SPEED, FINE_SPEED);
				break;
			case 3:
				oi_setWheels(FAST_SPEED, FAST_SPEED);
				timer_waitMillis(2000);
				break;
			case 4:
				loop = false;
				break;
			
			default:
				break;
		}

		oi_update(cyBot);
		dist += cyBot->distance;
		ang += cyBot->angle;

		lcd_printf("Cur Dist: %06.02lf\nProgress: %02.02lf/2.00m", dist, dist / 1000.0);
		

	}

	lcd_printf("Offset: %12.08lf\nAngle: %6.04lf", dist / 40000, ang);
	timer_waitMillis(100);

	while (button_getButton() != 0) {
		timer_waitMillis(10);
	}

	while (button_getButton() != 4) {
		timer_waitMillis(10);
	}
	lcd_printf("");

}
