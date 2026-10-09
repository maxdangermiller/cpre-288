/**
 * CyBot Scan Calibration
 * made for Lab 6, added here for lab 7 in lab 5 (aka Simple Mission)
 */
#include "cyBot_Scan_Cal.h"

/**
 * Estimate IR Distance
 */
double cyBot_est_IR_dist(double ir_dist) {
	// y = a exp(-cx) + b
	// a = 314.7
	// b = 10.23
	// c = 0.001943
	// RMSE = 1.565
	return 314.7 * exp(-0.001943 * ir_dist) + 10.23;


	// y = ax^b
	// a = 3794000
	// b = -1.613
	// RMSE: 1.697
	// return 3794000.0 * pow(ir_dist, -1.613);
}

void cyBot_init_Scan_cust(int feature) {
	cyBOT_init_Scan(feature);

	#ifdef USE_CYBOT_01
	// Values for CyBot #1
	right_calibration_value = 232750;
	left_calibration_value = 1225000;
	#endif
	
	#ifdef USE_CYBOT_11
	// CyBot #11
	right_calibration_value = 295750;
	left_calibration_value = 1288000;
	#endif
	
	#ifdef USE_CYBOT_17
	// CyBot #17
    right_calibration_value = 248500;
    left_calibration_value = 1204000;
	#endif

	#ifdef USE_CYBOT_22
	// CyBot #22
    right_calibration_value = 248500;
    left_calibration_value = 1267000;
	#endif

	#ifdef USE_CYBOT_24
	// CyBot #24
    right_calibration_value = 253750;
    left_calibration_value = 1235500;
	#endif

	#ifdef USE_CYBOT_25
	// CyBot #25
    right_calibration_value = 285250;
    left_calibration_value = 1246000;
	#endif

	#ifdef USE_CYBOT_26
	// CyBot #26
	right_calibration_value = 311500;
	left_calibration_value = 1309000;
	#endif
}
