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
#define PRECISION 0.5

// New position handling
#define Kp 0.04	
#define ROT_KP 2.0 		// mm/s per degree, used in rotateDegrees
#define TURN_KP 300.0 	// mm/s per radian, used in driveToPoint
#define Kpp 0.8
#define Kph 60.0
// #define Kpe 2.0

#define TICKS_TO_MM ((72.0 * M_PI) / 508.8)
#define TRACK_WIDTH 235.0

#define DEG_TO_RAD M_PI / 180.0
#define RAD_TO_DEG 180.0 / M_PI

#define POS_PREC 4.0		// Position Precision in mm
#define ROT_PREC 1.0 		// Rotation Precision degrees

#define NOM_SPEED 200.0		// Nominal Speed in mm/s

#define MAX_TURN 400.0 		// mm/s
#define MIN_ROT_SPEED 20 	// mm/s


// Store current position of the CyBot
static double pos_x = 0.0;          // mm
static double pos_y = 0.0;          // mm
static double pos_deg = 0.0;        // degrees
static double pos_rad = 0.0;		// radians

// static Pos_t target_pos;

static char str[80];


double prevLeftTicks = 0.0;
double prevRightTicks = 0.0;


/**
 * @brief Clamp a double value from min to max
 * 
 * @param value 
 * @param min 
 * @param max 
 * @return clamped double 
 */
double clamp(double value, double min, double max) {
    if(value < min) return min;
    if(value > max) return max;
    return value;
}


/**
 * @name Update Odometry
 * @brief Update the current position and angle global variables 
 * 		  given the information from the encoders
 * 
 * @param leftTicks from the cyBot
 * @param rightTicks from the cyBot
 */
void updateOdometry(oi_t *cyBot) {
    double deltaLeftTicks  = cyBot->leftEncoderCount  - prevLeftTicks;
    double deltaRightTicks = cyBot->rightEncoderCount - prevRightTicks;

    double dL = deltaLeftTicks  * TICKS_TO_MM;
    double dR = deltaRightTicks * TICKS_TO_MM;

    double dTheta = (dL - dR) / TRACK_WIDTH;
    double dCenter = (dL + dR) / 2.0;

    pos_x += dCenter * cos(pos_rad + dTheta / 2.0);
    pos_y += dCenter * sin(pos_rad + dTheta / 2.0);
    pos_rad += dTheta;

	while(pos_rad > M_PI) {
		pos_rad -= 2.0 * M_PI;
	}
	
	while(pos_rad < -M_PI) {
		pos_rad += 2.0 * M_PI;
	}

	pos_deg += pos_rad * RAD_TO_DEG;

    prevLeftTicks  = cyBot->leftEncoderCount;
    prevRightTicks = cyBot->rightEncoderCount;
}


/**
 * @name Drive To Absolute Point
 * @brief Drive to a point with error correction
 * 
 * @param cyBot 
 * @param targetX x-coordinate (mm)
 * @param targetY y-coordinate (mm)
 */
void driveToPoint(oi_t *cyBot, double targetX, double targetY) {
	// target_pos.x = targetX;
	// target_pos.y = targetY;
	oi_update(cyBot);
	prevLeftTicks = cyBot->leftEncoderCount;
	prevRightTicks = cyBot->rightEncoderCount;

	double dx, dy, dis, targetHeading, headingError, forward, turn;
	double leftVel, rightVel;

    while(true) {
        // Update Sensors
		oi_update(cyBot);

		// Update the position and rotation
        updateOdometry(cyBot);


        dx = targetX - pos_x;
        dy = targetY - pos_y;

        dis = sqrt(dx * dx + dy * dy);

        if(dis < POS_PREC) // within 10 mm
            break;

        targetHeading = atan2(dy, dx);

        sprintf(str, "Current position: (%.2lf mm, %.2lf mm, %.2lf deg)\t", pos_x, pos_y, pos_deg);
        cyBot_sendString(str);

        sprintf(str, "target: (%.2lf mm, %.2lf mm, %.2lf deg)\t", targetX, targetY, targetHeading);
        cyBot_sendString(str);

		
        headingError = targetHeading - pos_rad;
		
        while(headingError > M_PI) {
			headingError -= 2.0 * M_PI;
		}
		
        while(headingError < -M_PI) {
			headingError += 2.0 * M_PI;
		}
		
		
		// Slow down if not facing target
		forward = Kpp * dis * cos(headingError);
		
		// Slow down near target
		if (dis < 200.0) {
			forward *= dis / 100.0;
		}
		
		// Prevent driving hard while facing wrong way
		forward *= cos(headingError);
		
		if (dis > POS_PREC && fabs(forward) < 30) {
            forward = (forward >= 0) ? 30 : -30;
        }

		turn = clamp(Kph * headingError, -NOM_SPEED, NOM_SPEED);
		
        // Wheel speed matching
		/*
        double leftSpeed = leftTicks - prevLeftTicks;			// Left Speed is based off of the delta between the previous and current ticks
        double rightSpeed = rightTicks - prevRightTicks;		// Right Speed is based off of the delta between the previous and current ticks
		
        double encoderError = leftSpeed - rightSpeed;			// Encoder Error is the delta between the two encoders
        double encoderCorrection = Kpe * encoderError;			// Calculate encoder correction
		
        double lSpeed = forward - turn - encoderCorrection;		// Calculate left speed
        double rSpeed = forward + turn + encoderCorrection;		// Calculate right speed
		
        lSpeed = clamp(lSpeed, -NOM_SPEED, NOM_SPEED);
        rSpeed = clamp(rSpeed, -NOM_SPEED, NOM_SPEED);
		*/
	
		leftVel = (int)round(forward - turn);
		rightVel = (int)round(forward + turn);

		clamp(leftVel, -NOM_SPEED, NOM_SPEED);
		clamp(rightVel, -NOM_SPEED, NOM_SPEED);

		sprintf(str, "Raw Encoder Data: (left/right) = (%d, %d)\t", cyBot->leftEncoderCount, cyBot->rightEncoderCount);
		cyBot_sendString(str);

		sprintf(str, "Velocities: (left/right) = (%.2lf, %.2lf)\r\n", leftVel, rightVel);
		cyBot_sendString(str);

		oi_setWheels(leftVel, rightVel);
    }

	oi_setWheels(0,0);

}


/**
* @name Rotate Relative
* @brief Rotates robot relative to current heading
* @param cyBot
* @param degrees positive=CCW, negative=CW
*/
void rotateDegrees(oi_t *cyBot, double degrees) {

    double targetDeg = pos_deg + degrees;

    while (targetDeg >= 180.0)
        targetDeg -= 360.0;

    while (targetDeg < -180.0)
        targetDeg += 360.0;

    while (1) {

        oi_update(cyBot);

        updateOdometry(cyBot);


        double error = targetDeg - pos_deg;

        while (error > 180.0)
            error -= 360.0;

        while (error < -180.0)
            error += 360.0;

        if (fabs(error) < ROT_PREC)
            break;

        double turnVel = ROT_KP * error;

        if (fabs(turnVel) < MIN_ROT_SPEED) {
            if (turnVel >= 0)
                turnVel = MIN_ROT_SPEED;
            else
                turnVel = -MIN_ROT_SPEED;
        }

        turnVel = clamp(
            turnVel,
            -MAX_TURN,
             MAX_TURN);

        int leftVel  = (int)round(-turnVel);
        int rightVel = (int)round( turnVel);

        oi_setWheels(leftVel, rightVel);
    }

    oi_setWheels(0, 0);
}


/**
 * @name Move Absolute
 * @brief Moves the CyBot to Absolute coordinates
 * @param cyBot
 * @param x x-coordinate (mm)
 * @param y y-coordinate (mm)
 * @param do_avoid true/false
 */
void move_abs(oi_t *cyBot, double x, double y, int do_avoid) {
	// Needs to calculate an angle to turn to so that we can actually go to that position
	// Then turn to that angle
	// Then go the correct distance to get there

	// Update where our target is
	// target_pos.x = x;
	// target_pos.y = y;

    sprintf(str, "\r\nCurrent position: (%.2lf, %.2lf); target: (%.2lf, %.2lf)\r\n", pos_x, pos_y, x, y);
    cyBot_sendString(str);
	
	double rad = atan2(y - pos_y, x - pos_x);   // Angle to get there in radians
	double deg = rad * RAD_TO_DEG;	        	// Angle to get there in degrees

	sprintf(str, "Going to angle %.2lf rad or %.2lf deg\r\n", rad, deg);
	cyBot_sendString(str);

	double m_x = fabs(pos_x - x);			// Distance to move in the x direction
	double m_y = fabs(pos_y - y);			// Distance to move in the y direction
	double m_d = sqrt(m_x*m_x + m_y*m_y);	// Distance to move along the hypotenuse

	double dis_change;						// Var to store the change in each iteration
	double deg_change;						// Var to store the change in each iteration
	double change_x;						// Stores the change in the x direction in each loop
	double change_y;						// Stores the change in the y direction in each loop

	double error;

	int slow = 0;

	sprintf(str, "\r\nCurrent position: (%.2lf, %.2lf); target: (%.2lf, %.2lf)\r\n", pos_x, pos_y, x, y);
	cyBot_sendString(str);


	// If the CyBot needs to rotate to go to that position, go ahead and rotate it
	if (fabs(pos_deg - deg) > PRECISION) {
	    cyBot_sendString("\r\nTurning!\r\n\r\n");
		turn_abs(cyBot, deg);
		deg = pos_deg;
		rad = pos_rad;
	}

	sprintf(str, "Going to angle %.2lf=%.2lf rad or %.2lf deg\r\n", rad, pos_deg * DEG_TO_RAD, pos_deg);
	cyBot_sendString(str);

	// Set speed to FAST_SPEED
	oi_setWheels(FAST_SPEED, FAST_SPEED);

	sprintf(str, "Distance: %.2lf\r\n", m_d);
	cyBot_sendString(str);

	// While the distance left is greater than PRECISION, keep iterating
	while (!isnan(m_d) && m_d > PRECISION) {
		
		oi_update(cyBot);									// Update the CyBot Sensors
		
		dis_change = cyBot->distance * DIS_CALIBRATION;		// Set the distance (mm) since last update
		deg_change = cyBot->angle * ANG_CALIBRATION;		// Set the rotation (deg) since last update

		// Dynamic Proportional Correction
		error = (deg_change - atan2(y - pos_y, x - pos_x) * RAD_TO_DEG) * Kp;

		if (do_avoid && has_collided(cyBot)) {				// if do_avoid is enabled check if we have collided
			avoid(cyBot);										// Run Avoid Function
			continue;											// Start the movement loop over again
		}

		if (!slow) {
			oi_setWheels((int)(FAST_SPEED - error), (int)(FAST_SPEED + error)); // Reset Movement
		}
		else {
			oi_setWheels((int)(FINE_SPEED - error), (int)(FINE_SPEED + error)); // Reset Movement

		}



		// rad = atan2(y - pos_y, x - pos_x);
		deg += deg_change;
		pos_deg = deg;
		pos_rad = deg * DEG_TO_RAD;

		change_x = dis_change * cosf(pos_rad);				// Find change in the x direction
		change_y = dis_change * sinf(pos_rad); 				// Find change in the y direction


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
	
	pos_x += dis_change * cosf(rad);
	pos_y += dis_change * sinf(rad);

}


/**
 * @name Turn To Absolute Angle
 * @param cyBot
 * @param target_deg Target Rotation Degrees
 */
void turn_abs(oi_t *cyBot, double target_deg) {
    // If the angle is negative, the direction is negative (aka clock-wise),
    // otherwise the direction is positive (aka counter-clock-wise)
	int dir = (pos_deg - target_deg) < 0 ? -1 : 1;
	
	double ang_change;				// Var to store the change in each iteration

	int slow = 0;

	oi_setWheels(-dir * FAST_SPEED, dir * FAST_SPEED);


	sprintf(str, "Turning (abs): from %.2lf to %.2lf!\r\n", pos_deg, target_deg);
	cyBot_sendString(str);

	while (dir * (pos_deg - target_deg) > PRECISION) {
		oi_update(cyBot);

		// Set the change in the angle since the last update
		ang_change = (cyBot->angle * ANG_CALIBRATION);
		
		pos_deg += ang_change;
		pos_rad = pos_deg * DEG_TO_RAD; 

	    // sprintf(str, "Direction: %d, target: %.2lf, current: %.2lf, delta: %.2lf, change: %.2lf", dir, target_deg, pos_deg, pos_deg - target_deg, ang_change);
	    // cyBot_sendString(str);

	    // sprintf(str, ", prediction: %0.2lf\r\n", pos_deg - target_deg + 3 * ang_change);
	    // cyBot_sendString(str);

	    // If we are going to be there by there in the next three loops, slow down
        if (!slow && dir * (pos_deg - target_deg + 3 * ang_change) <= PRECISION) {
            cyBot_sendString("Slowing down\r\n");
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
	pos_rad = pos_deg * DEG_TO_RAD;

	sprintf(str, "Turning (abs): Final angle %.2lf where the target was %.2lf!\r\n", pos_deg, target_deg);
	cyBot_sendString(str);
}


/**
 * @name Move Forward
 * @param cyBot 
 * @param mm (millimeters)
 */
void move_forward(oi_t *cyBot, int mm) {
	// move_rel(cyBot, mm, true);
	// move_abs(cyBot, pos_x + (double)mm * cos(pos_rad), pos_y + (double)mm * sin(pos_rad), true);
	driveToPoint(cyBot, pos_x + (double)mm * cos(pos_rad), pos_y + (double)mm * sin(pos_rad));
	turn_abs(cyBot, 0);
}


/**
 * @name Move Backward
 * @param cyBot 
 * @param mm (millimeters)
 */
void move_backward(oi_t *cyBot, int mm) {
	// move_rel(cyBot, -mm, true);
	driveToPoint(cyBot, pos_x - (double)mm * cos(pos_rad), pos_y - (double)mm * sin(pos_rad));
	turn_abs(cyBot, 0);
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

	double rad = pos_deg * DEG_TO_RAD;

	double dis_change = 0.0;
	double dis_tot = 0.0;

	oi_setWheels(dir * FAST_SPEED, dir * FAST_SPEED);

	sprintf(str, "Distance: %d.00\r\n", mm);
	cyBot_sendString(str);

	int slow = 0;

	// dir * (mm - dis_tot) > PRECISION
	while (fabs(dir * mm - dir * dis_tot) > PRECISION) {
		oi_update(cyBot);									// Update the CyBot Sensors


		dis_change = cyBot->distance * DIS_CALIBRATION;	    // Update distance traveled
		dis_tot += dis_change;                              // Update total distance traveled

		if (do_avoid && has_collided(cyBot)) {				// if do_avoid is enabled check if we have collided
			avoid(cyBot);										// Run Avoid Function
			oi_setWheels(dir * FAST_SPEED, dir * FAST_SPEED);	// Reset Movement
			continue;											// Start the movement loop over again
		}

		sprintf(str, "Move Rel (update): distance: %.2lf, to go: %.2lf\r\n", dis_tot, dir * (mm - dis_tot));
		cyBot_sendString(str);

		if (dir * (mm - dis_tot) < 0.0) {
			dir = -dir;
			oi_setWheels(dir * FINE_SPEED, dir * FINE_SPEED);
		}

		// If we are going to be there by there in the next five loops, slow down
        if (!slow && dir * (mm - dis_tot - 5 * dis_change) <= PRECISION) {
            cyBot_sendString("Slowing down\r\n");
            oi_setWheels(dir * FINE_SPEED, dir * FINE_SPEED);
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
	// turn_abs(cyBot, pos_deg - degrees);
	rotateDegrees(cyBot, -degrees);
}


/**
 * @name Turn Counter-clockwise
 * @param cyBot
 * @param degrees
 */
void turn_ccw(oi_t *cyBot, int degrees) {
	// turn_abs(cyBot, pos_deg + degrees);
	rotateDegrees(cyBot, degrees);
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
