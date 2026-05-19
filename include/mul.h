#ifndef MUL_H
#define MUL_H
#include <stdint.h>
#include <inttypes.h>
#include "PE.h"
struct MUL{
    int32_t c;
};
extern struct MUL mul[16];
void mul_act(struct MUL *mul, int8_t a, int8_t b);
void mul_store(int8_t (*bram)[16], int row_indx);
#endif