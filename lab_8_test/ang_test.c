#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static double convert_s_ang(double a_s, double d_s) {
	a_s = (a_s - 90.0) * (M_PI / 180.0);

	return (atan(tan(a_s) - 10/(d_s * cos(a_s)))) * (180 / M_PI) + 90.0;
} 

int main(int argc, char const *argv[])
{
    int i;

    for (i = 0; i < 180; i += 2) 
    {
        printf("A_s = %d --> A_b = %lf\n", i, convert_s_ang(i, 30));
    }

    return 0;
}
