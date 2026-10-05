#include "cyBot_Scan_Cal.h"


extern volatile int button_event;

/**
 * Calibrate IR Distance
 */
void cyBot_IR_Calibrate() {
	static struct Calibration_Data {
		int dist_a;         // Actual distance (cm)
		double dist_m_ir;   // Measured distance (IR)
		double dist_m_s;   	// Measured distance (Sound)
	} calibration_data[24];
	
	static int distances[] = {9, 10, 12, 20, 30, 40, 50};

	char str[50];
	cyBOT_Scan_t data;
	int i,j;
	int dist;
	
	sprintf(str, "\r\nStarting IR Calibration\r\n\r\n");
	cyBot_sendString(str);

	// Go ahead and move the servo to 90 degrees
	cyBOT_Scan(90, &data);
	
	for (i = 0; i < 7; i++) {
		dist = distances[i];
		
		sprintf(str, "Place IR Sensor %dcm from object (press and button to measure)\r\n", dist);
		cyBot_sendString(str);
		
		// while (cyBot_getByte_blocking() != 'm');
		while (!button_event);

		
		for (j = 0; j < 3; j++) {
			cyBOT_Scan(90, &data);
			
			calibration_data[3 * i + j].dist_a = dist;
			calibration_data[3 * i + j].dist_m_ir = data.IR_raw_val;
			calibration_data[3 * i + j].dist_m_s = data.sound_dist;
		}
		
		timer_waitMillis(50); // Debounce
		button_event = 0;
	}
	
	sprintf(str, "All Data Collected. Sending Data in csv format.\r\n\r\n");
	cyBot_sendString(str);
	
	sprintf(str, "actual,ir,sound\r\n");
	cyBot_sendString(str);
	
	for (i = 0; i < 24; i++) {
		sprintf(str, "%d,%lf,%lf\r\n", calibration_data[i].dist_a, calibration_data[i].dist_m_ir, calibration_data[i].dist_m_s);
		cyBot_sendString(str);
	}

	sprintf(str, "\r\nCalibration Finished.\r\n");
	cyBot_sendString(str);
	
}

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
