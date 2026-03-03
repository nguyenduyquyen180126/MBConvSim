#ifndef PE_H
#define PE_H
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bram.h"
// ============== PE ===============
struct PE{
    int8_t ifm[16];
    int8_t weight[16];
    int32_t out;
};
/*
@brief Load dữ liệu từ BRAM vào PE
@param[in] ifm_bram_row Hàng để đọc dữ liệu từ bram
@param[in] w_row Hàng đọc dữ liêu từ bram
*/
void pe_load(struct PE *pe, int8_t (*ifm_bram)[16], int ifm_row, int8_t (*weight_bram)[16], int w_row){
    /*
        Copy ifm
        Copy weight
    */
    memcpy(pe->ifm, ifm_bram + ifm_row, BRAM_WIDTH_IN_BYTE);
    memcpy(pe->weight, weight_bram + w_row, BRAM_WIDTH_IN_BYTE);
}
void pe_compute(struct PE* pe){
    int32_t temp = 0;
    for(int i = 0; i < 16; i++){
        temp += pe->ifm[i] * pe->weight[i];
    }
    pe->out = temp;
}
/*
    PE debug
*/
void print_pe(struct PE *pe){
    printf("ifm:\n");
    for(int i = 0; i < 16; i++){
        printf("%4"PRId8" ", pe->ifm[i]);
    }
    printf("\n");
    printf("weight: \n");
    for(int i = 0; i < 16; i++){
        printf("%4"PRId8" ", pe->weight[i]);
    }
    printf("\n");
    printf("out: \n");
    printf("%"PRId32" ", pe->out);
}
// ================== PE Array =================
struct PE pe_array[16];
void pe_load_all(struct PE (*pe_array)[16], int ifm_row_to_load, int w_row_to_load){
    // Load IFM_BRAM
    for(int i = 0; i < 16; i++){
        pe_load()
    }
}
void pe_array_compute(struct PE (*pe_array)[16]){
    for(int i = 0; i < 16; i++){
        pe_compute(pe_array[i]);
    }
}
void pe_array_store(struct PE (*pe_array)[16], int bram_index){
    for(int i = 0; i < 16; i++){
        ACC_BRAM[bram_index][i] = pe_array[i]->out;
    }
}
void pe_array_reset_acc(struct PE (*pe_array)[16]){
    for(int i = 0; i < 16; i++){
        pe_array[i]->out = 0;
    }
}
#endif