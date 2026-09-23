#ifndef PART2_H

#define PART2_H

#include "cyBot_Scan.h"
#include "cyBot_uart.h"
#include <stdio.h>
#include <stdlib.h>

int part_2();


void do_scan(float* arr);
void send_scan(float* arr);
void cyBot_sendString(char* str);


#endif
