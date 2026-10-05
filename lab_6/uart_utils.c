#include "uart_utils.h"

/**
 * cyBot send string back to user
 * @param str
 */
void cyBot_sendString(char* str) {
	int i = -1;

	while (str[++i] != '\0') {
		cyBot_sendByte(str[i]);
	}
}
