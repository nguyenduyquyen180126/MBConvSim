#ifndef BRAM_H
#define BRAM_H
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "dram.h"
#include "config.h"

// ========================== Ping pong config ===========================
enum BRAM_STATE{
    WRITE,
    READ
};
int ping_state = READ;
int pong_state = WRITE;
int ping_start_row = 0;
int pong_start_row = 1175;
// =========================== 1. BRAM của PWConv ================================
/*
    BRAM(2352 x 16 pixels(8 bits)(128 bits))
    DMA bitwidths is 128 bits
*/

int8_t PWCONV_IFM_BRAM[8192][16];
int8_t PWCONV_W0_BRAM[2352][16];
int8_t PWCONV_W1_BRAM[2352][16];
int8_t PWCONV_W2_BRAM[2352][16];
int8_t PWCONV_W3_BRAM[2352][16];
int8_t PWCONV_W4_BRAM[2352][16];
int8_t PWCONV_W5_BRAM[2352][16];
int8_t PWCONV_W6_BRAM[2352][16];
int8_t PWCONV_W7_BRAM[2352][16];
int8_t PWCONV_W8_BRAM[2352][16];
int8_t PWCONV_W9_BRAM[2352][16];
int8_t PWCONV_W10_BRAM[2352][16];
int8_t PWCONV_W11_BRAM[2352][16];
int8_t PWCONV_W12_BRAM[2352][16];
int8_t PWCONV_W13_BRAM[2352][16];
int8_t PWCONV_W14_BRAM[2352][16];
int8_t PWCONV_W15_BRAM[2352][16];

int32_t PWCONV_ACC_BRAM[8192][16];
int8_t (*pwconv_w_brams[16])[16] = {
        PWCONV_W0_BRAM, PWCONV_W1_BRAM, PWCONV_W2_BRAM, PWCONV_W3_BRAM, PWCONV_W4_BRAM, PWCONV_W5_BRAM, PWCONV_W6_BRAM, PWCONV_W7_BRAM,
        PWCONV_W8_BRAM, PWCONV_W9_BRAM, PWCONV_W10_BRAM, PWCONV_W11_BRAM, PWCONV_W12_BRAM, PWCONV_W13_BRAM, PWCONV_W14_BRAM, PWCONV_W15_BRAM
    };


// ============================== 2. BRAM của DWConv ===============================
int8_t DW_IFM_BRAM[8192][16];
int8_t DW_W_BRAM[2352][16];
int32_t DW_ACC_BRAM[8192][16];

// ============================== 3. Helper function for bram ====================================
/*
@brief Hàm mô phòng việc load từ DRAM vào BRAM bằng DMA.
@param[in] dram Tên dram
@param[in] addr_dram Địa chỉ bắt đầu lấy dữ liệu theo byte dạng int8_t (0 lấy từ pixel dầu, 1 lấy từ pixel thứ 2)
@param[in] trans_size_in_byte Kích thước truyền đi in byte (phải nhỏ hơn bằng 16 bytes)
@param[in] bram Tên bram
@param[in] addr_bram Địa chỉ hàng bram
@return 1 nếu thành công, 0 nếu hỏng
*/
int load_bram(int8_t *dram, int addr_dram, int trans_size_in_byte, int8_t (*bram)[16], int addr_bram){
    if(trans_size_in_byte > 16){
        printf("[ERROR] DMA không truyen đuoc qua 128 bits\n");
        return SYS_INVALID_ARG;
    }
    memcpy(bram + addr_bram, dram + addr_dram, trans_size_in_byte);
    return 1;
}
/*
    Double buffering BRAM
*/

/*
@brief Debug purpose funtion
*/
void print_bram(int8_t (*bram)[16]){
    for(int i = 0; i < 9; i++){
        printf("%4d: ", i);
        for(int j = 0; j < 16; j++){
            printf("%4"PRId8" ", bram[i][j]);
        }
        printf("\n");
    }
    // printf("....\n");
    // for(int i = 2045; i < 2352; i++){
    //     printf("%4d: ", i);
    //     for(int j = 0; j < 16; j++){
    //         printf("%4"PRId8" ", bram[i][j]);
    //     }
    //     printf("\n");
    // }
    printf("\n");

}
/*
@brief Only for int32_t
*/
int print_bram_to_file(const char *file_name, int32_t (*bram)[16], int num_of_row){
    FILE *f = fopen(file_name, "w");
    if(f == NULL){
        printf("[ERROR] Khong viet duoc bram vao file\n");
        return SYS_ERROR;
    }
    for(int i = 0; i < num_of_row; i++){
        for(int j = 0; j < 16; j++){
            fprintf(f, "%"PRId32"\n", bram[i][j]);
        }
    }
    fclose(f);
    printf("[LOGS] Viet thanh cong\n");
    return SYS_OK;
}
int print_bram_to_file_int8(const char *file_name, int8_t (*bram)[16], int width, int depth){
    FILE *f = fopen(file_name, "w");
    if(f == NULL){
        printf("[ERROR] Khong viet duoc bram vao file\n");
        return SYS_ERROR;
    }
    for(int i = 0; i < depth; i++){
        for(int j = 0; j < width; j++){
            fprintf(f, "%"PRId8"\n", bram[i][j]);
        }
    }
    printf("[ERROR] Viet thanh cong bram vao file\n");
    return SYS_OK;
}
#endif