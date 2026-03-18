#ifndef PE_H
#define PE_H
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bram.h"
#include "config.h"
// ================= 1. PE của depthwise conv ===================
struct PWCONV_PE{
    int8_t ifm[16];
    int8_t weight[16];
    int32_t out;
};
/*
@brief Load dữ liệu từ BRAM vào PE
@param[in] ifm_bram_row Hàng để đọc dữ liệu từ bram
@param[in] w_row Hàng đọc dữ liêu từ bram
*/
void pw_pe_load(struct PWCONV_PE *pe, int8_t (*ifm_bram)[16], int ifm_row, int8_t (*weight_bram)[16], int w_row){
    /*
        Copy ifm
        Copy weight
    */
    memcpy(pe->ifm, ifm_bram + ifm_row, BRAM_WIDTH_IN_BYTE);
    memcpy(pe->weight, weight_bram + w_row, BRAM_WIDTH_IN_BYTE);
}
// void pe_compute(struct PE* pe){
//     int32_t temp = 0;
//     for(int i = 0; i < 16; i++){
//         temp += pe->ifm[i] * pe->weight[i];
//     }
//     pe->out += temp;
// }
/*
    Hàm load và tính các PE
*/
void pw_pe_compute(struct PWCONV_PE *pe, int8_t (*ifm_bram)[16], int ifm_row, int8_t (*weight_bram)[16], int w_row){
    memcpy(pe->ifm, ifm_bram + ifm_row, BRAM_WIDTH_IN_BYTE);
    memcpy(pe->weight, weight_bram + w_row, BRAM_WIDTH_IN_BYTE);
    int32_t temp = 0;
    for(int i = 0; i < 16; i++){
        temp += pe->ifm[i] * pe->weight[i];
    }
    pe->out += temp;
    
}
/*
    PE debug
*/
void pw_pe_print(struct PWCONV_PE *pe){
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
// PE Array 
struct PWCONV_PE pw_pe_array[16];

void pw_pe_array_store(struct PWCONV_PE *pe_array, int bram_index){
    for(int i = 0; i < 16; i++){
        PWCONV_ACC_BRAM[bram_index][i] = pe_array[i].out;
    }
}
void pw_pe_array_reset_acc(struct PWCONV_PE *pe_array){
    for(int i = 0; i < 16; i++){
        pe_array[i].out = 0;
    }
}
/*
    Debug acc
*/
void print_acc_bram(int32_t (*bram)[16]){
    for(int i = 0; i < 9; i++){
        printf("%4d: ", i);
        for(int j = 0; j < 16; j++){
            printf("%4"PRId32" ", bram[i][j]);
        }
        printf("\n");
    }
    printf("....\n");
}
// =================== 2. PE của depthwise =================
struct DW_PE{
    int8_t ifm;
    int8_t weight;
    int32_t acc;
};
void dw_pe_compute(struct DW_PE *pe, int8_t ifm, int8_t weight){
    pe->acc += ifm * weight;
}


struct DW_PE dw_pe_arr[16];
void dw_pe_arr_reset(){
    for(int i = 0; i < 16; i++){
        dw_pe_arr[i].acc = 0;
    }
}
void dw_pe_arr_store(struct DW_PE *pe_array, int acc_row_addr){
    for(int i = 0; i < 16; i++){
        DW_ACC_BRAM[acc_row_addr][i] = pe_array[i].acc;
    }
}
// ================== 3. PE của SE_PW ====================
struct SE_PW{
    int8_t ifm[16];
    int8_t weight[16];
    int32_t acc;
};
void se_pw_compute(struct SE_PW * pe, int8_t (*ifm)[16], int ifm_row, int8_t (*weight)[16], int w_row){
    memcpy(pe->ifm, ifm + ifm_row, BRAM_WIDTH_IN_BYTE);
    memcpy(pe->weight, weight + w_row, BRAM_WIDTH_IN_BYTE);
    for(int i = 0; i < 16; i++){
        pe->acc += pe->ifm[i] + pe->weight[i];
    }
}

struct SE_PW se_pw_1_arr[4];
struct SE_PW se_pw_2_arr[4];
void se_pw_reset(struct SE_PW *pe_arr){
    for(int i = 0; i < 4; i++){
        pe_arr[i].acc = 0;
    }
}
void se_pw_store(struct SE_PW *pe_arr, int32_t (*acc_bram_addr)[16], int row_addr){
    for(int i = 0; i < 16; i++){
        acc_bram_addr[row_addr][i] = pe_arr[i].acc;
    }
}

#endif