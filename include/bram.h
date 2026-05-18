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

extern int ping_start_row;
extern int pong_start_row;
// =========================== 1. BRAM của PWConv ================================
/*
    BRAM(1152 x 16 pixels(8 bits)(128 bits))
    DMA bitwidths is 128 bits
*/

extern int8_t PWCONV_IFM_BRAM[16384][16];
extern int8_t PWCONV_W0_BRAM[1152][16];
extern int8_t PWCONV_W1_BRAM[1152][16];
extern int8_t PWCONV_W2_BRAM[1152][16];
extern int8_t PWCONV_W3_BRAM[1152][16];
extern int8_t PWCONV_W4_BRAM[1152][16];
extern int8_t PWCONV_W5_BRAM[1152][16];
extern int8_t PWCONV_W6_BRAM[1152][16];
extern int8_t PWCONV_W7_BRAM[1152][16];
extern int8_t PWCONV_W8_BRAM[1152][16];
extern int8_t PWCONV_W9_BRAM[1152][16];
extern int8_t PWCONV_W10_BRAM[1152][16];
extern int8_t PWCONV_W11_BRAM[1152][16];
extern int8_t PWCONV_W12_BRAM[1152][16];
extern int8_t PWCONV_W13_BRAM[1152][16];
extern int8_t PWCONV_W14_BRAM[1152][16];
extern int8_t PWCONV_W15_BRAM[1152][16];

extern int8_t PWCONV_ACC_BRAM[16384][16];
extern int8_t (*pwconv_w_brams[16])[16];


// ============================== 2. BRAM của DWConv ===============================
extern int8_t DW_W_BRAM[1152][16];
extern int8_t DW_ACC_BRAM[16384][16];

// ============================== 3. BRAM của GAP ==============================
extern int8_t GAP_BRAM[1152][16];

// ============================== 4. BRAM của SE PW1 =========================
extern int8_t SE_PW_1_W1_BRAM[1152][16];
extern int8_t SE_PW_1_W2_BRAM[1152][16];
extern int8_t SE_PW_1_W3_BRAM[1152][16];
extern int8_t SE_PW_1_W4_BRAM[1152][16];

extern int8_t (*se_pw_1_w_brams[4])[16];
extern int8_t SE_PW_1_ACC_BRAM[16384][16];
extern int8_t (*SE_PW_1_ACC_4_WIDTH_BRAM)[4];
// ============================== 5. BRAM của SE PW2 =========================
extern int8_t SE_PW_2_W1_BRAM[1152][16];
extern int8_t SE_PW_2_W2_BRAM[1152][16];
extern int8_t SE_PW_2_W3_BRAM[1152][16];
extern int8_t SE_PW_2_W4_BRAM[1152][16];
extern int8_t (*se_pw_2_w_brams[4])[16];
extern int8_t SE_PW_2_ACC_BRAM[16384][16];
extern int8_t (*SE_PW_2_ACC_4_WIDTH_BRAM)[4];
// ============================== 5. BRAM của MUL =============================
extern int8_t MUL_BRAM[16384][16];
// ============================== 6. BRAM của PW_LAST =============================
extern int8_t PW_LAST_W0_BRAM[1152][16];
extern int8_t PW_LAST_W1_BRAM[1152][16];
extern int8_t PW_LAST_W2_BRAM[1152][16];
extern int8_t PW_LAST_W3_BRAM[1152][16];
extern int8_t PW_LAST_W4_BRAM[1152][16];
extern int8_t PW_LAST_W5_BRAM[1152][16];
extern int8_t PW_LAST_W6_BRAM[1152][16];
extern int8_t PW_LAST_W7_BRAM[1152][16];
extern int8_t PW_LAST_W8_BRAM[1152][16];
extern int8_t PW_LAST_W9_BRAM[1152][16];
extern int8_t PW_LAST_W10_BRAM[1152][16];
extern int8_t PW_LAST_W11_BRAM[1152][16];
extern int8_t PW_LAST_W12_BRAM[1152][16];
extern int8_t PW_LAST_W13_BRAM[1152][16];
extern int8_t PW_LAST_W14_BRAM[1152][16];
extern int8_t PW_LAST_W15_BRAM[1152][16];
extern int8_t (*pw_last_w_brams[16])[16];
extern int8_t PW_LAST_ACC_BRAM[16384][16];
// ========================== Usage Tracking ===========================
extern int max_row_ifm;
extern int max_row_pw_acc;
extern int max_row_dw_acc;
extern int max_row_gap;
extern int max_row_se1_acc;
extern int max_row_se2_acc;
extern int max_row_mul;
extern int max_row_pw_last_acc;
extern int max_row_output;

// Ping-pong weight tracking
extern int max_row_pw_w_ping;
extern int max_row_pw_w_pong;
extern int max_row_dw_w;
extern int max_row_se1_w_ping;
extern int max_row_se1_w_pong;
extern int max_row_se2_w_ping;
extern int max_row_se2_w_pong;
extern int max_row_pw_last_w_ping;
extern int max_row_pw_last_w_pong;

void update_max(int *max_var, int current_row);

// ========================== Ping pong config ===========================
/*
@brief Hàm mô phòng việc load từ DRAM vào BRAM bằng DMA.
@param[in] dram Tên dram
@param[in] addr_dram Địa chỉ bắt đầu lấy dữ liệu theo byte dạng int8_t (0 lấy từ pixel dầu, 1 lấy từ pixel thứ 2)
@param[in] trans_size_in_byte Kích thước truyền đi in byte (phải nhỏ hơn bằng 16 bytes)
@param[in] bram Tên bram
@param[in] addr_bram Địa chỉ hàng bram
@return 1 nếu thành công, 0 nếu hỏng
*/
int load_bram(int8_t *dram, int addr_dram, int trans_size_in_byte, int8_t (*bram)[16], int addr_bram);

void report_bram_usage();

void __load_bram(int8_t *dram, int addr_dram, int trans_size_in_byte, int8_t (*bram)[16], int addr_bram);
/*
    Double buffering BRAM
*/

/*
@brief Debug purpose funtion
*/
void print_bram(int8_t (*bram)[16]);
void print_bram_32_bit(int32_t (*bram)[16]);
/*
@brief Only for int8_t
*/
int print_bram_to_file(const char *file_name, int8_t (*bram)[16], int num_of_row);
int print_bram_to_file_int8(const char *file_name, int8_t (*bram)[16], int width, int depth);


#endif