#ifndef GAP_H
#define GAP_H
#include <stdint.h>
#include <inttypes.h>

int32_t gap_acc[16]; // Bộ cộng tích luỹ

void gap_acc_reset(){
    for(int i = 0; i < 16; i++){
        gap_acc[i] = 0;
    }
}
void gap_acc_store(int32_t *gap_acc, int8_t (*gap_bram)[16], int row_addr){
    for(int i = 0; i < 16; i++){
        gap_bram[row_addr][i] = (int8_t)gap_acc[i];
    }
}
#endif