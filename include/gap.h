#ifndef GAP_H
#define GAP_H
#include <stdint.h>
#include <inttypes.h>

extern int32_t gap_acc[16]; // Bộ cộng tích luỹ

void gap_acc_reset();
void gap_acc_store(int32_t *gap_acc, int8_t (*gap_bram)[16], int row_addr);
#endif