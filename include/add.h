#ifndef ADD_H
#define ADD_H
#include <stdint.h>
#include <inttypes.h>
#include "PE.h"
extern int8_t OUTPUT[16384][16];
struct ADD{
    int8_t a;
    int8_t b;
    int32_t c;
};
extern struct ADD add_arr[16];
void add_compute(struct ADD *add, int8_t a, int8_t b);
void add_reset();
void add_store(int row_addr);
#endif