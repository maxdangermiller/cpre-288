#include "part2.h"


#define SCAN_RESOLUTION 2
#define MIN_ANGLE 0
#define MAX_ANGLE 180
#define SCAN_STEPS (MAX_ANGLE - MIN_ANGLE) / SCAN_RESOLUTION + 1


int part_2() {
	cyBot_uart_init();
	cyBOT_init_Scan(0b111);

	char c;

	float* scan_data = (float*)malloc(sizeof(float) * SCAN_STEPS);

	while (1) {
		c = (char)cyBot_getByte();

		if (c == 'm') {
			do_scan(scan_data);

			
		}

		if (c == 'e') {
            break;


        }

		send_scan(scan_data);
	}
	

	return 0;
}


void do_scan(float* arr) {
	int i;

	cyBOT_Scan_t data;

	for (i = 0; i < SCAN_STEPS; i++) {
		cyBOT_Scan(i * SCAN_RESOLUTION + MIN_ANGLE, &data);
		
		arr[i] = data.sound_dist;
	}
}

void send_scan(float* arr) {
	
	int i, j;
	char str[35];

	sprintf(str, "Angle(Degrees)\tDistance(cm)\r\n");

	cyBot_sendString(str);

	for(i = 0; i < SCAN_STEPS; i++) {
		// Print Data Line
		sprintf(str, "%d\t\t\t%lf\r\n", 2*i, arr[i]);

		cyBot_sendString(str);
	}
}

void cyBot_sendString(char* str) {
	int i = -1;

	while (str[++i] != '\0') {
		cyBot_sendByte(str[i]);
	}
}
