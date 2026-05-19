#ifndef DRAM_H
#define DRAM_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "config.h"

#define DRAM_SIZE 1000000
extern int8_t DRAM[DRAM_SIZE];
/*
    Mảng 1 chiều lớn chứa cả IFM, W, B
    0-18815 IFM C->W->H
    18816-end W Cin->H->W->Cout
*/

int init_dram(const char *file_name, int8_t *dram);
/*
@brief Debug purpose function
*/
void print_dram(int8_t *dram);
#endif