#include "cyBot_Scan.h"
#include "cyBot_uart.h"
#include <stdio.h>
#include <stdlib.h>

#define SCAN_RESOLUTION 2
#define MIN_ANGLE 0
#define MAX_ANGLE 180
#define SCAN_STEPS (MAX_ANGLE - MIN_ANGLE) / SCAN_RESOLUTION


void do_scan(float* arr);
void send_scan(float* arr);
void cyBot_sendString(char* str);

int main(int argc, char const *argv[]) {
    cyBot_uart_init();
    cyBOT_init_Scan(0b111);

    char c;

    float* scan_data = (float*)malloc(sizeof(float) * SCAN_STEPS);

    while (1) {
        c = (char)cyBot_getByte();

        if (c == 'm') {
            do_scan(scan_data);

            
        }
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
    char str[30];

    sprintf(str, "Angle(Degrees)	Distance(m)\n");

    cyBot_sendString(str);

	for(i = 0; i <= 90; i++) {
		// Print Data Line
		sprintf(str, "%d\t\t\t%lf\n", 2*i, arr[i]);

        cyBot_sendString(str);
	}
}

void cyBot_sendString(char* str) {
    int i = -1;

    while (str[++i] != '\0') {
        cyBot_sendByte(str[i]);
    }
}