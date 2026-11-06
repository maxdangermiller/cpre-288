#ifndef PING_H
#define PING_H

void ping_init(void);
int ping_read(void);
int ping_get_overflows(void);
float ping_cycles_to_ms(int cycles);
float ping_cycles_to_cm(int cycles);


#endif
