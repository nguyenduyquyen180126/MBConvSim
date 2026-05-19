#include "bram.h"





unsigned long long total_load_cycles = 0;
const int DMA_LATENCY = 30; // Giả sử mỗi lần load mất 30 cycles (có thể điều chỉnh tùy theo thực tế)







int ping_start_row = 0;
int pong_start_row = 576;
// =========================== 1. BRAM của PWConv ================================
/*
    BRAM(1152 x 16 pixels(8 bits)(128 bits))
    DMA bitwidths is 128 bits
*/

int8_t PWCONV_IFM_BRAM[16384][16];
int8_t PWCONV_W0_BRAM[1152][16];
int8_t PWCONV_W1_BRAM[1152][16];
int8_t PWCONV_W2_BRAM[1152][16];
int8_t PWCONV_W3_BRAM[1152][16];
int8_t PWCONV_W4_BRAM[1152][16];
int8_t PWCONV_W5_BRAM[1152][16];
int8_t PWCONV_W6_BRAM[1152][16];
int8_t PWCONV_W7_BRAM[1152][16];
int8_t PWCONV_W8_BRAM[1152][16];
int8_t PWCONV_W9_BRAM[1152][16];
int8_t PWCONV_W10_BRAM[1152][16];
int8_t PWCONV_W11_BRAM[1152][16];
int8_t PWCONV_W12_BRAM[1152][16];
int8_t PWCONV_W13_BRAM[1152][16];
int8_t PWCONV_W14_BRAM[1152][16];
int8_t PWCONV_W15_BRAM[1152][16];

int8_t PWCONV_ACC_BRAM[16384][16];
int8_t (*pwconv_w_brams[16])[16] = {
        PWCONV_W0_BRAM, PWCONV_W1_BRAM, PWCONV_W2_BRAM, PWCONV_W3_BRAM, PWCONV_W4_BRAM, PWCONV_W5_BRAM, PWCONV_W6_BRAM, PWCONV_W7_BRAM,
        PWCONV_W8_BRAM, PWCONV_W9_BRAM, PWCONV_W10_BRAM, PWCONV_W11_BRAM, PWCONV_W12_BRAM, PWCONV_W13_BRAM, PWCONV_W14_BRAM, PWCONV_W15_BRAM
    };


// ============================== 2. BRAM của DWConv ===============================
int8_t DW_W_BRAM[1152][16];
int8_t DW_ACC_BRAM[16384][16];

// ============================== 3. BRAM của GAP ==============================
int8_t GAP_BRAM[1152][16];

// ============================== 4. BRAM của SE PW1 =========================
int8_t SE_PW_1_W1_BRAM[1152][16];
int8_t SE_PW_1_W2_BRAM[1152][16];
int8_t SE_PW_1_W3_BRAM[1152][16];
int8_t SE_PW_1_W4_BRAM[1152][16];

int8_t (*se_pw_1_w_brams[4])[16] = {
    SE_PW_1_W1_BRAM, SE_PW_1_W2_BRAM, SE_PW_1_W3_BRAM, SE_PW_1_W4_BRAM
};
int8_t SE_PW_1_ACC_BRAM[16384][16];
int8_t (*SE_PW_1_ACC_4_WIDTH_BRAM)[4] = (int8_t (*)[4])SE_PW_1_ACC_BRAM;
// ============================== 5. BRAM của SE PW2 =========================
int8_t SE_PW_2_W1_BRAM[1152][16];
int8_t SE_PW_2_W2_BRAM[1152][16];
int8_t SE_PW_2_W3_BRAM[1152][16];
int8_t SE_PW_2_W4_BRAM[1152][16];
int8_t (*se_pw_2_w_brams[4])[16] = {
    SE_PW_2_W1_BRAM, SE_PW_2_W2_BRAM, SE_PW_2_W3_BRAM, SE_PW_2_W4_BRAM
};
int8_t SE_PW_2_ACC_BRAM[16384][16];
int8_t (*SE_PW_2_ACC_4_WIDTH_BRAM)[4] = (int8_t (*)[4])SE_PW_2_ACC_BRAM;
// ============================== 5. BRAM của MUL =============================
int8_t MUL_BRAM[16384][16];
// ============================== 6. BRAM của PW_LAST =============================
int8_t PW_LAST_W0_BRAM[1152][16];
int8_t PW_LAST_W1_BRAM[1152][16];
int8_t PW_LAST_W2_BRAM[1152][16];
int8_t PW_LAST_W3_BRAM[1152][16];
int8_t PW_LAST_W4_BRAM[1152][16];
int8_t PW_LAST_W5_BRAM[1152][16];
int8_t PW_LAST_W6_BRAM[1152][16];
int8_t PW_LAST_W7_BRAM[1152][16];
int8_t PW_LAST_W8_BRAM[1152][16];
int8_t PW_LAST_W9_BRAM[1152][16];
int8_t PW_LAST_W10_BRAM[1152][16];
int8_t PW_LAST_W11_BRAM[1152][16];
int8_t PW_LAST_W12_BRAM[1152][16];
int8_t PW_LAST_W13_BRAM[1152][16];
int8_t PW_LAST_W14_BRAM[1152][16];
int8_t PW_LAST_W15_BRAM[1152][16];
int8_t (*pw_last_w_brams[16])[16] = {
        PW_LAST_W0_BRAM, PW_LAST_W1_BRAM, PW_LAST_W2_BRAM, PW_LAST_W3_BRAM, PW_LAST_W4_BRAM, PW_LAST_W5_BRAM, PW_LAST_W6_BRAM, PW_LAST_W7_BRAM,
        PW_LAST_W8_BRAM, PW_LAST_W9_BRAM, PW_LAST_W10_BRAM, PW_LAST_W11_BRAM, PW_LAST_W12_BRAM, PW_LAST_W13_BRAM, PW_LAST_W14_BRAM, PW_LAST_W15_BRAM
    };
int8_t PW_LAST_ACC_BRAM[16384][16];
// ========================== Usage Tracking ===========================
int max_row_ifm = 0;
int max_row_pw_acc = 0;
int max_row_dw_acc = 0;
int max_row_gap = 0;
int max_row_se1_acc = 0;
int max_row_se2_acc = 0;
int max_row_mul = 0;
int max_row_pw_last_acc = 0;
int max_row_output = 0;

// Ping-pong weight tracking
int max_row_pw_w_ping = 0;
int max_row_pw_w_pong = 0;
int max_row_dw_w = 0;
int max_row_se1_w_ping = 0;
int max_row_se1_w_pong = 0;
int max_row_se2_w_ping = 0;
int max_row_se2_w_pong = 0;
int max_row_pw_last_w_ping = 0;
int max_row_pw_last_w_pong = 0;

void update_max(int *max_var, int current_row) {
    if (current_row > *max_var) *max_var = current_row;
}


int load_bram(int8_t *dram, int addr_dram, int trans_size_in_byte, int8_t (*bram)[16], int addr_bram){
    if(trans_size_in_byte > 16){
        printf("[ERROR] DMA không truyen đuoc qua 128 bits\n");
        return SYS_INVALID_ARG;
    }
    memcpy(bram + addr_bram, dram + addr_dram, trans_size_in_byte);
    total_load_cycles += DMA_LATENCY + 1;
    return 1;
}

void report_bram_usage() {
    printf("\n[BRAM USAGE REPORT - MAX ROWS ACCESSED]\n");
    printf("IFM BRAM:         %d rows\n", max_row_ifm + 1);
    printf("PW Weight (Ping/Pong): %d / %d rows\n", max_row_pw_w_ping + 1, max_row_pw_w_pong + 1);
    printf("PW ACC BRAM:      %d rows\n", max_row_pw_acc + 1);
    printf("DW Weight:             %d rows\n", max_row_dw_w + 1);
    printf("DW ACC BRAM:      %d rows\n", max_row_dw_acc + 1);
    printf("GAP BRAM:         %d rows\n", max_row_gap + 1);
    printf("SE1 Weight (Ping/Pong):%d / %d rows\n", max_row_se1_w_ping + 1, max_row_se1_w_pong + 1);
    printf("SE1 ACC BRAM:     %d rows\n", max_row_se1_acc + 1);
    printf("SE2 Weight (Ping/Pong):%d / %d rows\n", max_row_se2_w_ping + 1, max_row_se2_w_pong + 1);
    printf("SE2 ACC BRAM:     %d rows\n", max_row_se2_acc + 1);
    printf("MUL BRAM:         %d rows\n", max_row_mul + 1);
    printf("PW LAST W (Ping/Pong): %d / %d rows\n", max_row_pw_last_w_ping + 1, max_row_pw_last_w_pong + 1);
    printf("PW LAST ACC BRAM: %d rows\n", max_row_pw_last_acc + 1);
    printf("OUTPUT BRAM:      %d rows\n", max_row_output + 1);
}

void __load_bram(int8_t *dram, int addr_dram, int trans_size_in_byte, int8_t (*bram)[16], int addr_bram){
    int8_t *bram_flatten = (int8_t *)bram;
    memcpy(bram_flatten + addr_bram, dram + addr_dram, trans_size_in_byte);
}

void print_bram(int8_t (*bram)[16]){
    for(int i = 0; i < 9; i++){
        printf("%4d: ", i);
        for(int j = 0; j < 16; j++){
            printf("%4" PRId8 " ", bram[i][j]);
        }
        printf("\n");
    }
    printf("\n");

}
void print_bram_32_bit(int32_t (*bram)[16]){
    for(int i = 0; i < 9; i++){
        printf("%4d: ", i);
        for(int j = 0; j < 16; j++){
            printf("%4" PRId32 " ", bram[i][j]);
        }
        printf("\n");
    }
    printf("\n");

}

int print_bram_to_file(const char *file_name, int8_t (*bram)[16], int num_of_row){
    FILE *f = fopen(file_name, "w");
    if(f == NULL){
        printf("[ERROR] Khong viet duoc bram vao file\n");
        return SYS_ERROR;
    }
    for(int i = 0; i < num_of_row; i++){
        for(int j = 0; j < 16; j++){
            fprintf(f, "%" PRId8 "\n", bram[i][j]);
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
            fprintf(f, "%" PRId8 "\n", bram[i][j]);
        }
    }
    fclose(f);
    printf("[LOGS] Viet thanh cong bram vao file\n");
    return SYS_OK;
}