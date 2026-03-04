#ifndef BRAM_H
#define BRAM_H
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "dram.h"
#include "config.h"

/*
    BRAM(2048 x 16 pixels(8 bits)(128 bits))
    DMA bitwidths is 128 bits
*/
#define DMA_BW 128
#define BRAM_WIDTH_IN_BIT 128
#define BRAM_WIDTH_IN_BYTE 16
#define PIXEL_DAT_SIZE 8

int8_t IFM_BRAM[2048][16];
int8_t W0_BRAM[2048][16];
int8_t W1_BRAM[2048][16];
int8_t W2_BRAM[2048][16];
int8_t W3_BRAM[2048][16];
int8_t W4_BRAM[2048][16];
int8_t W5_BRAM[2048][16];
int8_t W6_BRAM[2048][16];
int8_t W7_BRAM[2048][16];
int8_t W8_BRAM[2048][16];
int8_t W9_BRAM[2048][16];
int8_t W10_BRAM[2048][16];
int8_t W11_BRAM[2048][16];
int8_t W12_BRAM[2048][16];
int8_t W13_BRAM[2048][16];
int8_t W14_BRAM[2048][16];
int8_t W15_BRAM[2048][16];

int32_t ACC_BRAM[8192][16];
int8_t (*w_brams[16])[16] = {
        W0_BRAM, W1_BRAM, W2_BRAM, W3_BRAM, W4_BRAM, W5_BRAM, W6_BRAM, W7_BRAM,
        W8_BRAM, W9_BRAM, W10_BRAM, W11_BRAM, W12_BRAM, W13_BRAM, W14_BRAM, W15_BRAM
    };

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
    printf("....\n");
    // for(int i = 2045; i < 2048; i++){
    //     printf("%4d: ", i);
    //     for(int j = 0; j < 16; j++){
    //         printf("%4"PRId8" ", bram[i][j]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");

}
/*
@brief Only for int32_t
*/
int print_bram_to_file(const char *file_name, int32_t (*bram)[16]){
    FILE *f = fopen(file_name, "w");
    if(f == NULL){
        printf("[ERROR] Khong viet duoc bram vao file");
        return SYS_ERROR;
    }
    for(int i = 0; i < 14 * 14 * 384 / 16; i++){
        for(int j = 0; j < 16; j++){
            fprintf(f, "%"PRId32"\n", bram[i][j]);
        }
    }
    fclose(f);
    printf("[LOGS] Viet thanh cong");
    return SYS_OK;
}

// int main(){
//     if(init_dram("ifm.txt", DRAM) == SYS_OK){
//         printf("[LOGS] Read file successfully\n");
//     }
//     print_dram(DRAM);
//     load_bram(DRAM, 0, PIXEL_DAT_SIZE * 16, IFM_BRAM, 6);
//     print_bram(IFM_BRAM);
// }
#endif