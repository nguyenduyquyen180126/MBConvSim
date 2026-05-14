#ifndef MUL_H
#define MUL_H
#include <stdint.h>
#include <inttypes.h>
struct MUL{
    int32_t c;
};
struct MUL mul[16];
void mul_act(struct MUL *mul, int8_t a, int8_t b){
    mul->c = (int32_t)a * (int32_t)b;
}
void mul_store(int8_t (*bram)[16], int row_indx){
    for(int i = 0; i < 16; i++){
        bram[row_indx][i] = (int8_t)mul[i].c;
    }
}
#endif