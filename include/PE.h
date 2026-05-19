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
void pw_pe_load(struct PWCONV_PE *pe, int8_t (*ifm_bram)[16], int ifm_row, int8_t (*weight_bram)[16], int w_row);
/*
    Hàm load và tính các PE
*/
void pw_pe_compute(struct PWCONV_PE *pe, int8_t (*ifm_bram)[16], int ifm_row, int8_t (*weight_bram)[16], int w_row);
/*
    PE debug
*/
void pw_pe_print(struct PWCONV_PE *pe);
// ========================== PEConv Array ========================= 
extern struct PWCONV_PE pw_pe_array[16];

void pw_pe_array_store(struct PWCONV_PE *pe_array, int8_t (*acc_bram)[16], int bram_index);
void pw_pe_array_store_to_bram(struct PWCONV_PE *pe_array, int8_t (*bram)[16], int bram_index);
void pw_pe_array_reset_acc(struct PWCONV_PE *pe_array);
/*
    Debug acc
*/
void pw_pe_array_store_to_bram(struct PWCONV_PE *pe_array, int8_t (*bram)[16], int bram_index);
void pw_pe_array_reset_acc(struct PWCONV_PE *pe_array);
/*
    Debug acc
*/
void print_acc_bram(int8_t (*bram)[16]);
// =================== 2. PE của depthwise =================
struct DW_PE{
    int8_t ifm;
    int8_t weight;
    int32_t acc;
};
void dw_pe_compute(struct DW_PE *pe, int8_t ifm, int8_t weight);


extern struct DW_PE dw_pe_arr[16];
void dw_pe_arr_reset();
void dw_pe_arr_store(struct DW_PE *pe_array, int acc_row_addr);
// ================== 3. PE của SE_PW ====================
extern struct PWCONV_PE se_pw_pe_1_arr[4];
extern struct PWCONV_PE se_pw_pe_2_arr[4];
void se_pw_reset(struct PWCONV_PE *pe_arr);
void se_pw_store(struct PWCONV_PE *pe_arr, int8_t (*acc_bram)[16], int bram_addr);
// ====================== 4. PE của PW_LAST ===================
extern struct PWCONV_PE pw_last_pe_arr[16];
#endif