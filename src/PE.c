#include "PE.h"
void pw_pe_load(struct PWCONV_PE *pe, int8_t (*ifm_bram)[16], int ifm_row, int8_t (*weight_bram)[16], int w_row){
    /*
        Copy ifm
        Copy weight
    */
    memcpy(pe->ifm, ifm_bram + ifm_row, BRAM_WIDTH_IN_BYTE);
    memcpy(pe->weight, weight_bram + w_row, BRAM_WIDTH_IN_BYTE);
}

void pw_pe_compute(struct PWCONV_PE *pe, int8_t (*ifm_bram)[16], int ifm_row, int8_t (*weight_bram)[16], int w_row){
    memcpy(pe->ifm, ifm_bram + ifm_row, BRAM_WIDTH_IN_BYTE);
    memcpy(pe->weight, weight_bram + w_row, BRAM_WIDTH_IN_BYTE);
    int32_t temp = 0;
    for(int i = 0; i < 16; i++){
        temp += pe->ifm[i] * pe->weight[i];
    }
    pe->out += temp;
    
}

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
 
struct PWCONV_PE pw_pe_array[16];

void pw_pe_array_store(struct PWCONV_PE *pe_array, int8_t (*acc_bram)[16], int bram_index){
    if (acc_bram == PWCONV_ACC_BRAM) update_max(&max_row_pw_acc, bram_index);
    else if (acc_bram == PW_LAST_ACC_BRAM) update_max(&max_row_pw_last_acc, bram_index);
    
    for(int i = 0; i < 16; i++){
        acc_bram[bram_index][i] = (int8_t)pe_array[i].out;
    }
}
void pw_pe_array_store_to_bram(struct PWCONV_PE *pe_array, int8_t (*bram)[16], int bram_index){
    update_max(&max_row_pw_last_acc, bram_index);
    for(int i = 0; i < 16; i++){
        bram[bram_index][i] = (int8_t)pe_array[i].out;
    }
}
void pw_pe_array_reset_acc(struct PWCONV_PE *pe_array){
    for(int i = 0; i < 16; i++){
        pe_array[i].out = 0;
    }
}

void print_acc_bram(int8_t (*bram)[16]){
    for(int i = 0; i < 9; i++){
        printf("%4d: ", i);
        for(int j = 0; j < 16; j++){
            printf("%4"PRId8" ", bram[i][j]);
        }
        printf("\n");
    }
    printf("....\n");
}

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
    update_max(&max_row_dw_acc, acc_row_addr);
    for(int i = 0; i < 16; i++){
        DW_ACC_BRAM[acc_row_addr][i] = (int8_t)pe_array[i].acc;
    }
}

struct PWCONV_PE se_pw_pe_1_arr[4];
struct PWCONV_PE se_pw_pe_2_arr[4];
void se_pw_reset(struct PWCONV_PE *pe_arr){
    for(int i = 0; i < 4; i++){
        pe_arr[i].out = 0;
    }
}
void se_pw_store(struct PWCONV_PE *pe_arr, int8_t (*acc_bram)[16], int bram_addr){
    if (acc_bram == SE_PW_1_ACC_BRAM) update_max(&max_row_se1_acc, (bram_addr + 3) / 16);
    else if (acc_bram == SE_PW_2_ACC_BRAM) update_max(&max_row_se2_acc, (bram_addr + 3) / 16);
    int8_t *acc_bram_flatten = (int8_t *)acc_bram;
    for(int i = 0; i < 4; i++){
        acc_bram_flatten[bram_addr + i] = (int8_t)pe_arr[i].out;
    }
}

struct PWCONV_PE pw_last_pe_arr[16];