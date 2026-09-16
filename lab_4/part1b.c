#include "stdio.h"
#include "sensor-data.h"


int main() {
	// Define interator variable
	int i;

	// Print Header
	printf("Angle(Degrees)	Distance(m)\n");

	for(i=0; i<=90; i++) {
		// Print Data Line
		printf("%d\t\t\t%lf\n", 2*i, sensor_data_array[i]);
	}

	return 0;
}
