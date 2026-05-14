#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dram.h"
#include "bram.h"
#include "PE.h"
#include <stdint.h>
#include "config.h"
#include <omp.h>
#include <unistd.h>
#include "gap.h"
#include "mul.h"
#include "add.h"

int PW_C_IN = 96;
int PW_C_OUT = 384;
int PARALLEL = 16;
int NUM_OF_PE = 16;
int NUM_OF_BRAM = 16;
int PW_H_in = 14;
int PW_W_in = 14;
int PW_H_out = 14;
int PW_W_out = 14;
int PW_FILTER_SIZE = 96;
int PW_FILTER_DEPTH = 96;
int PW_NUM_OF_FILTER = 384;
int DMA_BW = 128;
int BRAM_WIDTH_IN_BIT = 128;
int BRAM_WIDTH_IN_BYTE = 16;
int PIXEL_DAT_SIZE = 8;

int PW_WEIGHT_START_ADDR = 96 * 14 * 14;
int DW_H_IN = 14;
int DW_W_IN = 14;
int DW_C_IN = 384;
int DW_H_OUT = 14;
int DW_W_OUT = 14;
int DW_C_OUT = 384;
int DW_H_K = 3;
int DW_W_K = 3;
int DW_C_K = 1;
int DW_NUM_OF_K = 384;
int DW_STRIDE = 1;

int NUM_OF_SE_PE = 4;
int NUM_OF_SE_BRAM = 4;
int SE_PW_1_CIN = 384;
int SE_PW_1_COUT = 24;

int SE_PW_2_CIN = 24;
int SE_PW_2_COUT = 384;

int PW_LAST_CIN = 384;
int PW_LAST_H = 14;
int PW_LAST_W = 14;
int PW_LAST_COUT = 96;

void parse_args(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            char *name = argv[i] + 1;
            if (i + 1 < argc) {
                int val = atoi(argv[i+1]);
                if (strcmp(name, "PW_C_IN") == 0) PW_C_IN = val;
                else if (strcmp(name, "PW_C_OUT") == 0) PW_C_OUT = val;
                else if (strcmp(name, "PARALLEL") == 0) PARALLEL = val;
                else if (strcmp(name, "NUM_OF_PE") == 0) NUM_OF_PE = val;
                else if (strcmp(name, "NUM_OF_BRAM") == 0) NUM_OF_BRAM = val;
                else if (strcmp(name, "PW_H_in") == 0) PW_H_in = val;
                else if (strcmp(name, "PW_W_in") == 0) PW_W_in = val;
                else if (strcmp(name, "PW_H_out") == 0) PW_H_out = val;
                else if (strcmp(name, "PW_W_out") == 0) PW_W_out = val;
                else if (strcmp(name, "PW_FILTER_SIZE") == 0) PW_FILTER_SIZE = val;
                else if (strcmp(name, "PW_FILTER_DEPTH") == 0) PW_FILTER_DEPTH = val;
                else if (strcmp(name, "PW_NUM_OF_FILTER") == 0) PW_NUM_OF_FILTER = val;
                else if (strcmp(name, "DMA_BW") == 0) DMA_BW = val;
                else if (strcmp(name, "BRAM_WIDTH_IN_BIT") == 0) BRAM_WIDTH_IN_BIT = val;
                else if (strcmp(name, "BRAM_WIDTH_IN_BYTE") == 0) BRAM_WIDTH_IN_BYTE = val;
                else if (strcmp(name, "PIXEL_DAT_SIZE") == 0) PIXEL_DAT_SIZE = val;
                else if (strcmp(name, "DW_H_IN") == 0) DW_H_IN = val;
                else if (strcmp(name, "DW_W_IN") == 0) DW_W_IN = val;
                else if (strcmp(name, "DW_C_IN") == 0) DW_C_IN = val;
                else if (strcmp(name, "DW_H_OUT") == 0) DW_H_OUT = val;
                else if (strcmp(name, "DW_W_OUT") == 0) DW_W_OUT = val;
                else if (strcmp(name, "DW_C_OUT") == 0) DW_C_OUT = val;
                else if (strcmp(name, "DW_H_K") == 0) DW_H_K = val;
                else if (strcmp(name, "DW_W_K") == 0) DW_W_K = val;
                else if (strcmp(name, "DW_C_K") == 0) DW_C_K = val;
                else if (strcmp(name, "DW_NUM_OF_K") == 0) DW_NUM_OF_K = val;
                else if (strcmp(name, "DW_STRIDE") == 0) DW_STRIDE = val;
                else if (strcmp(name, "NUM_OF_SE_PE") == 0) NUM_OF_SE_PE = val;
                else if (strcmp(name, "NUM_OF_SE_BRAM") == 0) NUM_OF_SE_BRAM = val;
                else if (strcmp(name, "SE_PW_1_CIN") == 0) SE_PW_1_CIN = val;
                else if (strcmp(name, "SE_PW_1_COUT") == 0) SE_PW_1_COUT = val;
                else if (strcmp(name, "SE_PW_2_CIN") == 0) SE_PW_2_CIN = val;
                else if (strcmp(name, "SE_PW_2_COUT") == 0) SE_PW_2_COUT = val;
                else if (strcmp(name, "PW_LAST_CIN") == 0) PW_LAST_CIN = val;
                else if (strcmp(name, "PW_LAST_H") == 0) PW_LAST_H = val;
                else if (strcmp(name, "PW_LAST_W") == 0) PW_LAST_W = val;
                else if (strcmp(name, "PW_LAST_COUT") == 0) PW_LAST_COUT = val;
                i++;
            }
        }
    }
    // Update derived values
    PW_WEIGHT_START_ADDR = PW_C_IN * PW_H_in * PW_W_in;
}

int main(int argc, char *argv[]){
    parse_args(argc, argv);
    omp_set_max_active_levels(2);
    printf("====================================== Bat dau ========================================\n");
    // ======================= 1. Mô phỏng của lớp point wise conv =====================
    // ============= Load to dram ==============
    if(init_dram("test/dram.txt", DRAM) == SYS_OK){
        printf("[LOGS] Read file successfully\n");
    } else {
        printf("[LOGS] Failed to read file\n");
        return -1;
    }
    // print_dram(DRAM);
    
    // ============== Load IFM_BRAM ================
    printf("[LOGS] Load IFM BRAM\n");
    for(int i = 0; i < PW_H_in * PW_W_in * PW_C_IN / BRAM_WIDTH_IN_BYTE; i++){ // i - số hàng BRAM
        load_bram(DRAM, i * BRAM_WIDTH_IN_BYTE, BRAM_WIDTH_IN_BYTE, PWCONV_IFM_BRAM, i);
    }
    // print_bram(PWCONV_IFM_BRAM);
    printf("[LOGS] IFM BRAM Loaded.\n");
    
    // ============== Load PW weight ================
    // Nap san du lieu vao w bram de tinh duoc tile dau
    printf("[LOGS] Loading PW Weight BRAMs...\n");
    uint16_t write_enable_weight = 1;
    while(write_enable_weight != 0){ // Vòng for ngoài cùng duyệt các BRAM
        int bram_indx = __builtin_ctz(write_enable_weight);// Load BRAM thứ mấy

        for(int i = 0; i < PW_C_IN / BRAM_WIDTH_IN_BYTE; i++){
            load_bram(DRAM, PW_WEIGHT_START_ADDR + i * BRAM_WIDTH_IN_BYTE + bram_indx * PW_FILTER_SIZE, BRAM_WIDTH_IN_BYTE, pwconv_w_brams[bram_indx], i);
        }

        write_enable_weight = write_enable_weight << 1;
    }

    printf("[LOGS] PW Weight BRAMs Loaded.\n");
    // ================ Load DW weight ===============
    printf("[LOGS] Loading DW Weight BRAM...\n");

    int dw_start_addr = PW_H_in * PW_W_in * PW_C_IN + PW_NUM_OF_FILTER * PW_FILTER_SIZE;
    for(int i = 0; i < DW_H_K * DW_W_K * DW_NUM_OF_K / BRAM_WIDTH_IN_BYTE; i++){
        load_bram(DRAM, dw_start_addr + i*BRAM_WIDTH_IN_BYTE, BRAM_WIDTH_IN_BYTE, DW_W_BRAM, i);
    }
    printf("[LOGS] DW Weight BRAM Loaded\n");

    // =================== Kich thuoc padding ===============
    int total_pad_i = (DW_H_OUT - 1) * DW_STRIDE + DW_H_K - DW_H_IN;
    int pad_top = total_pad_i / 2;
    int pad_bot = total_pad_i - pad_top;

    int total_pad_j = (DW_W_OUT - 1) * DW_STRIDE + DW_W_K - DW_W_IN;
    int pad_left = total_pad_j / 2;
    int pad_right = total_pad_j - pad_left;

    // =================== Load SE PW1 =====================
    // Load truoc 4 cai filter vao cac BRAM
    printf("[LOGS] Starting SE PW1 load weight...\n");
    int se_pw_1_start_addr = dw_start_addr + DW_H_K * DW_W_K * DW_NUM_OF_K;
    int se_pw_block_size = (SE_PW_1_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE;
    for(int bram_indx = 0; bram_indx < NUM_OF_SE_BRAM; bram_indx++){
        for(int row_indx = 0; row_indx < se_pw_block_size; row_indx++){ // row_indx goes from 0 to 23 (384/16)
            int dram_addr = se_pw_1_start_addr + bram_indx * SE_PW_1_CIN + row_indx * BRAM_WIDTH_IN_BYTE;
            load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, se_pw_1_w_brams[bram_indx], row_indx);
        }
    }
    printf("[LOGS] SE PW1 loaded\n");
    // ==================== Load SE PW2 =====================
    printf("[LOGS] Starting SE PW2 load weight...\n");
    int se_pw_2_start_addr = se_pw_1_start_addr + SE_PW_1_COUT * SE_PW_1_CIN;
    se_pw_block_size = (SE_PW_2_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE;
    for(int bram_indx = 0; bram_indx < NUM_OF_SE_BRAM; bram_indx++){
        for(int row_indx = 0; row_indx < se_pw_block_size; row_indx++){ // row_indx = 0, 1
            int dram_addr = se_pw_2_start_addr + bram_indx * SE_PW_2_CIN + row_indx * BRAM_WIDTH_IN_BYTE;
            load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, se_pw_2_w_brams[bram_indx], row_indx);
        }
    }
    printf("[LOGS] SE PW2 loaded\n");
    

    // ==================== Load PW LAST ====================
    int pw_last_start_addr = se_pw_2_start_addr + SE_PW_2_COUT * SE_PW_2_CIN;
    printf("[LOGS] Loading PW Weight BRAMs...\n");
   
    for(int bram_indx = 0; bram_indx < NUM_OF_BRAM; bram_indx++){
        for(int i = 0; i < PW_LAST_CIN / BRAM_WIDTH_IN_BYTE; i++){
             // Input channel block i, Output channel bram_indx
            int dram_addr = pw_last_start_addr + i * BRAM_WIDTH_IN_BYTE + bram_indx * PW_LAST_CIN;
            load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, pw_last_w_brams[bram_indx], ping_start_row + i);
        }
    }
    printf("[LOGS] PW Weight BRAMs Loaded.\n");

    // =================== Bien cho pipeline ===================
    printf("[LOGS] Starting PW-DW pipeline computation loops...\n");
    volatile int pw_row_compete = 0;
    volatile int dw_pixel_complete = 0;

    // =============== Pileline 3 stage: PW - DW - GAP =================
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            // ========================================= PW =========================================
            int compute_done = 1;
            int bram_load_done = 1;
            int ping_state = READ;
            int pong_state = WRITE;

            printf("[LOGS] Starting PW computation loops...\n");
            for(int tile = 0; tile < PW_NUM_OF_FILTER / NUM_OF_PE; tile++){ // Tính song song 16 kênh do đó chỉ cần tính C_OUT / PARALLEL lần.
                while(compute_done == 0 || bram_load_done == 0){
                    usleep(1);
                }
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        compute_done = 0;
                        int pingpong_row_loaded = 0;
                        for(int ho = 0; ho < PW_H_out; ho++){
                            for(int wo = 0; wo < PW_W_out; wo++){
                                pw_pe_array_reset_acc(pw_pe_array);
                                int row_needed_for_one_pixel_depth = PW_C_IN / BRAM_WIDTH_IN_BYTE; // Một vector 1x1xC_in.
                                int ifm_row_indx = (ho * PW_W_in + wo) * row_needed_for_one_pixel_depth;
                                
                                int w_row_indx = (ping_state == READ) ? ping_start_row : pong_start_row;
                                
                                // ================ Tinh va load =================
                                
                                for(int i = 0; i < PW_FILTER_SIZE / NUM_OF_PE; i++){
                                    // Song song viec load va tinh toan
                                    pw_pe_compute(&pw_pe_array[0], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W0_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[1], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W1_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[2], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W2_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[3], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W3_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[4], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W4_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[5], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W5_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[6], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W6_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[7], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W7_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[8], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W8_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[9], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W9_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[10], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W10_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[11], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W11_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[12], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W12_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[13], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W13_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[14], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W14_BRAM, w_row_indx + i);
                                    pw_pe_compute(&pw_pe_array[15], PWCONV_IFM_BRAM, ifm_row_indx + i, PWCONV_W15_BRAM, w_row_indx + i);
                                }
                            
                                // ================================================
                                int row_needed_for_one_pixel_depth_output = PW_C_OUT / BRAM_WIDTH_IN_BYTE;
                                int acc_row = (ho * PW_W_out + wo) * row_needed_for_one_pixel_depth_output + tile; //(Hàng bắt đầu + số hàng offset)
                                pw_pe_array_store(pw_pe_array, PWCONV_ACC_BRAM, acc_row);
                            }
                            #pragma omp atomic update
                            pw_row_compete++;
                        }
                        compute_done = 1;
                    }
                    #pragma omp section
                    {
                        bram_load_done = 0;
                        if(tile < (PW_NUM_OF_FILTER / NUM_OF_PE) - 1){
                            int w_row_indx_to_write = (ping_state == WRITE) ? ping_start_row : pong_start_row;
                            int rows_per_bram = PW_C_IN / BRAM_WIDTH_IN_BYTE;
                            int next_tile_offset = (tile + 1) * NUM_OF_PE * PW_FILTER_SIZE;
                            
                            for(int bram_indx = 0; bram_indx < NUM_OF_BRAM; bram_indx++){
                                for(int r = 0; r < rows_per_bram; r++){
                                    int dram_addr = PW_WEIGHT_START_ADDR + next_tile_offset + bram_indx * PW_FILTER_SIZE + r * BRAM_WIDTH_IN_BYTE;
                                    load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, pwconv_w_brams[bram_indx], w_row_indx_to_write + r);
                                }
                            }
                        }
                        bram_load_done = 1;
                    }
                }
                // Hoan doi vai tro ping va pong
                ping_state = 1 - ping_state;
                pong_state = 1 - pong_state;
            }
            printf("[LOGS] PWConv Done.\n");
            fflush(stdout);

            print_bram_to_file("output/acc.txt", PWCONV_ACC_BRAM, PW_H_out * PW_W_out * PW_C_OUT / BRAM_WIDTH_IN_BYTE);
        }
        #pragma omp section
        {
            // ====================================== DW ======================================
            printf("[LOGS] Starting DW computation loops...\n");
            for(int tile = 0; tile < DW_C_OUT / NUM_OF_PE; tile++){
                for(int ho = 0; ho < DW_H_OUT; ho++){
                    
                    int max_needed_row = ho * DW_STRIDE + (DW_H_K - 1) - pad_top;
                    if(max_needed_row >= PW_H_out) max_needed_row = PW_H_out - 1;
                    
                    int needed_compete_row = tile * PW_H_out + max_needed_row;
                    while(pw_row_compete <= needed_compete_row){
                        usleep(1);
                    }

                    for(int wo = 0; wo < DW_W_OUT; wo++){
                        dw_pe_arr_reset();
                        
                        for(int i = 0; i  < DW_H_K * DW_W_K; i++){
                            int k_i = i / DW_W_K;
                            int k_j = i % DW_W_K;

                            int ifm_i_indx = ho * DW_STRIDE + k_i - pad_top;
                            int ifm_j_indx = wo * DW_STRIDE + k_j - pad_left;

                            // Load to PE
                            int weight_row_addr = (k_i * DW_W_K + k_j) * (DW_NUM_OF_K / NUM_OF_PE) + tile;
                            int8_t *weight_row = DW_W_BRAM[weight_row_addr];

                            if(ifm_i_indx >= 0 && ifm_i_indx < DW_H_IN && ifm_j_indx >= 0 && ifm_j_indx < DW_W_IN){
                                // Tìm hàng dựa trên toạ độ i, j
                                int ifm_row_addr = (ifm_i_indx * DW_W_IN + ifm_j_indx) * (DW_C_IN / NUM_OF_PE) + tile;
                                int32_t *ifm_row_32 = PWCONV_ACC_BRAM[ifm_row_addr];
                                int8_t ifm_row[16];
                                for(int i = 0; i < 16; i++){
                                    ifm_row[i] = (int8_t)ifm_row_32[i];
                                }

                                dw_pe_compute(&dw_pe_arr[0], ifm_row[0], weight_row[0]);
                                dw_pe_compute(&dw_pe_arr[1], ifm_row[1], weight_row[1]);
                                dw_pe_compute(&dw_pe_arr[2], ifm_row[2], weight_row[2]);
                                dw_pe_compute(&dw_pe_arr[3], ifm_row[3], weight_row[3]);
                                dw_pe_compute(&dw_pe_arr[4], ifm_row[4], weight_row[4]);
                                dw_pe_compute(&dw_pe_arr[5], ifm_row[5], weight_row[5]);
                                dw_pe_compute(&dw_pe_arr[6], ifm_row[6], weight_row[6]);
                                dw_pe_compute(&dw_pe_arr[7], ifm_row[7], weight_row[7]);
                                dw_pe_compute(&dw_pe_arr[8], ifm_row[8], weight_row[8]);
                                dw_pe_compute(&dw_pe_arr[9], ifm_row[9], weight_row[9]);
                                dw_pe_compute(&dw_pe_arr[10], ifm_row[10], weight_row[10]);
                                dw_pe_compute(&dw_pe_arr[11], ifm_row[11], weight_row[11]);
                                dw_pe_compute(&dw_pe_arr[12], ifm_row[12], weight_row[12]);
                                dw_pe_compute(&dw_pe_arr[13], ifm_row[13], weight_row[13]);
                                dw_pe_compute(&dw_pe_arr[14], ifm_row[14], weight_row[14]);
                                dw_pe_compute(&dw_pe_arr[15], ifm_row[15], weight_row[15]);

                            }
                            else{
                                int8_t ifm_row[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Zp = 0

                                dw_pe_compute(&dw_pe_arr[0], ifm_row[0], weight_row[0]);
                                dw_pe_compute(&dw_pe_arr[1], ifm_row[1], weight_row[1]);
                                dw_pe_compute(&dw_pe_arr[2], ifm_row[2], weight_row[2]);
                                dw_pe_compute(&dw_pe_arr[3], ifm_row[3], weight_row[3]);
                                dw_pe_compute(&dw_pe_arr[4], ifm_row[4], weight_row[4]);
                                dw_pe_compute(&dw_pe_arr[5], ifm_row[5], weight_row[5]);
                                dw_pe_compute(&dw_pe_arr[6], ifm_row[6], weight_row[6]);
                                dw_pe_compute(&dw_pe_arr[7], ifm_row[7], weight_row[7]);
                                dw_pe_compute(&dw_pe_arr[8], ifm_row[8], weight_row[8]);
                                dw_pe_compute(&dw_pe_arr[9], ifm_row[9], weight_row[9]);
                                dw_pe_compute(&dw_pe_arr[10], ifm_row[10], weight_row[10]);
                                dw_pe_compute(&dw_pe_arr[11], ifm_row[11], weight_row[11]);
                                dw_pe_compute(&dw_pe_arr[12], ifm_row[12], weight_row[12]);
                                dw_pe_compute(&dw_pe_arr[13], ifm_row[13], weight_row[13]);
                                dw_pe_compute(&dw_pe_arr[14], ifm_row[14], weight_row[14]);
                                dw_pe_compute(&dw_pe_arr[15], ifm_row[15], weight_row[15]);

                            }
                        }
                        int acc_bram_row_addr = (ho * DW_W_OUT + wo) * (DW_C_OUT / NUM_OF_PE) + tile;
                        dw_pe_arr_store(dw_pe_arr, acc_bram_row_addr);
                        #pragma omp atomic update
                        dw_pixel_complete++;
                    }
                }
            }
            printf("[LOGS] DONE DW loop\n");
            print_bram_to_file("output/dw_acc.txt", DW_ACC_BRAM, DW_H_OUT * DW_W_OUT * DW_C_OUT / BRAM_WIDTH_IN_BYTE);

        }
        #pragma omp section
        {
            // ========================================== GAP ==========================================
            
            printf("[LOGS] Starting Global Average Pooling ...\n");
            for(int tile = 0; tile < DW_C_OUT / NUM_OF_PE; tile++){
                gap_acc_reset();
                for(int i = 0; i < DW_H_OUT * DW_W_OUT; i++){
                    
                    while(dw_pixel_complete <= i + tile * DW_H_OUT * DW_W_OUT){
                        usleep(1);
                    }

                    // Lay hang ra tu BRAM DW
                    int ifm_row_indx = i * (DW_C_OUT / NUM_OF_PE) + tile;
                    int32_t *ifm_32_bit = DW_ACC_BRAM[ifm_row_indx];
                    int8_t ifm[16];
                    for(int i = 0; i < 16; i++){
                        ifm[i] = (int8_t)ifm_32_bit[i];
                    }

                    gap_acc[0] += ifm[0];
                    gap_acc[1] += ifm[1];
                    gap_acc[2] += ifm[2];
                    gap_acc[3] += ifm[3];
                    gap_acc[4] += ifm[4];
                    gap_acc[5] += ifm[5];
                    gap_acc[6] += ifm[6];
                    gap_acc[7] += ifm[7];
                    gap_acc[8] += ifm[8];
                    gap_acc[9] += ifm[9];
                    gap_acc[10] += ifm[10];
                    gap_acc[11] += ifm[11];
                    gap_acc[12] += ifm[12];
                    gap_acc[13] += ifm[13];
                    gap_acc[14] += ifm[14];
                    gap_acc[15] += ifm[15];

                }
                gap_acc[0] /= DW_W_OUT * DW_H_OUT;
                gap_acc[1] /= DW_W_OUT * DW_H_OUT;
                gap_acc[2] /= DW_W_OUT * DW_H_OUT;
                gap_acc[3] /= DW_W_OUT * DW_H_OUT;
                gap_acc[4] /= DW_W_OUT * DW_H_OUT;
                gap_acc[5] /= DW_W_OUT * DW_H_OUT;
                gap_acc[6] /= DW_W_OUT * DW_H_OUT;
                gap_acc[7] /= DW_W_OUT * DW_H_OUT;
                gap_acc[8] /= DW_W_OUT * DW_H_OUT;
                gap_acc[9] /= DW_W_OUT * DW_H_OUT;
                gap_acc[10] /= DW_W_OUT * DW_H_OUT;
                gap_acc[11] /= DW_W_OUT * DW_H_OUT;
                gap_acc[12] /= DW_W_OUT * DW_H_OUT;
                gap_acc[13] /= DW_W_OUT * DW_H_OUT;
                gap_acc[14] /= DW_W_OUT * DW_H_OUT;
                gap_acc[15] /= DW_W_OUT * DW_H_OUT;
                
                gap_acc_store(gap_acc, GAP_BRAM, tile);
            }
            print_bram_to_file_int8("output/gap_acc.txt", GAP_BRAM, 16, DW_C_OUT / BRAM_WIDTH_IN_BYTE);
            printf("[LOGS] Done global average pooling\n");
        }

    }
        // ====================================== SE PW 1 ======================================
        printf("[LOGS] Starting SE Pointwise 1 Conv\n");
        int compute_done = 1;
        int bram_load_done = 1;
        int ping_state = READ;
        int pong_state = WRITE;

        
        for(int chuck_ofm = 0; chuck_ofm < SE_PW_1_COUT / NUM_OF_SE_PE; chuck_ofm++){

            int row_start_to_read = (ping_state == READ) ? ping_start_row : pong_start_row;
            int row_start_to_write = (ping_state == WRITE) ? ping_start_row : pong_start_row;
            se_pw_reset(se_pw_pe_1_arr);
            while(compute_done == 0 || bram_load_done == 0){
                usleep(1);
            }
            #pragma omp parallel sections
            {
                #pragma omp section
                {

                    for(int row_ifm = 0; row_ifm < (SE_PW_1_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE; row_ifm++){
                        
                        pw_pe_compute(&se_pw_pe_1_arr[0], GAP_BRAM, row_ifm, SE_PW_1_W1_BRAM, row_start_to_read + row_ifm);
                        pw_pe_compute(&se_pw_pe_1_arr[1], GAP_BRAM, row_ifm, SE_PW_1_W2_BRAM, row_start_to_read + row_ifm);
                        pw_pe_compute(&se_pw_pe_1_arr[2], GAP_BRAM, row_ifm, SE_PW_1_W3_BRAM, row_start_to_read + row_ifm);
                        pw_pe_compute(&se_pw_pe_1_arr[3], GAP_BRAM, row_ifm, SE_PW_1_W4_BRAM, row_start_to_read + row_ifm);
                    }
                    
                }
                #pragma omp section
                {
                    // Load Section
                    if(chuck_ofm + 1 < SE_PW_1_COUT / NUM_OF_SE_PE){
                        for(int bram_indx = 0; bram_indx < NUM_OF_SE_BRAM; bram_indx++){
                            for(int row = 0; row < (SE_PW_1_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE; row++){
                                int dram_addr = se_pw_1_start_addr + (chuck_ofm + 1) * SE_PW_1_CIN * NUM_OF_SE_BRAM  + bram_indx * SE_PW_1_CIN + row * BRAM_WIDTH_IN_BYTE;
                                load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, se_pw_1_w_brams[bram_indx], row_start_to_write + row);
                            }
                        }
                    }
                }
            }
            se_pw_store(se_pw_pe_1_arr, SE_PW_1_ACC_BRAM, chuck_ofm * 4);
            
            ping_state = 1 - ping_state;
            pong_state = 1 - pong_state;
        }
        printf("[LOGS] Done SE Pointwise 1 Conv\n");
        print_bram_to_file("output/se_pw1.txt", SE_PW_1_ACC_BRAM, (SE_PW_1_COUT + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE);

        // ============================================== SE PW 2 ==============================================
        compute_done = 1;
        bram_load_done = 1;
        printf("[LOGS] Starting SE Pointwise 2 Conv\n");

        ping_state = READ;
        pong_state = WRITE;
        
        for(int row_ofm = 0; row_ofm < SE_PW_2_COUT / NUM_OF_SE_PE; row_ofm++){

            int row_start_to_read = (ping_state == READ) ? ping_start_row : pong_start_row;
            int row_start_to_write = (ping_state == WRITE) ? ping_start_row : pong_start_row;
            se_pw_reset(se_pw_pe_2_arr);
            
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp atomic write
                    compute_done = 0;
                    // Compute Section
                    for(int row_ifm = 0; row_ifm < (SE_PW_2_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE; row_ifm++){
                        int needed_steps = (row_ifm + 1) * (BRAM_WIDTH_IN_BYTE / NUM_OF_SE_PE);
                        if(needed_steps > (SE_PW_1_COUT / NUM_OF_SE_PE)) needed_steps = (SE_PW_1_COUT / NUM_OF_SE_PE);
                        
                        
                        int32_t *ifm_32_bit = SE_PW_1_ACC_BRAM[row_ifm];
                        int8_t ifm_row[16];
                        for(int i = 0; i < 16; i++){
                            ifm_row[i] = (int8_t)ifm_32_bit[i];
                        }

                        pw_pe_compute(&se_pw_pe_2_arr[0], &ifm_row, 0, SE_PW_2_W1_BRAM, row_start_to_read + row_ifm);
                        pw_pe_compute(&se_pw_pe_2_arr[1], &ifm_row, 0, SE_PW_2_W2_BRAM, row_start_to_read + row_ifm);
                        pw_pe_compute(&se_pw_pe_2_arr[2], &ifm_row, 0, SE_PW_2_W3_BRAM, row_start_to_read + row_ifm);
                        pw_pe_compute(&se_pw_pe_2_arr[3], &ifm_row, 0, SE_PW_2_W4_BRAM, row_start_to_read + row_ifm);
                    }
                    #pragma omp atomic write
                    compute_done = 1;
                }
                #pragma omp section
                {
                    #pragma omp atomic write
                    bram_load_done = 0;
                    // Load Section
                    if(row_ofm + 1 < SE_PW_2_COUT / NUM_OF_SE_PE){
                        for(int bram_indx = 0; bram_indx < NUM_OF_SE_BRAM; bram_indx++){
                            for(int row = 0; row < (SE_PW_2_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE; row++){
                                int dram_addr = se_pw_2_start_addr + (row_ofm + 1) * SE_PW_2_CIN * NUM_OF_SE_BRAM  + bram_indx * SE_PW_2_CIN + row * BRAM_WIDTH_IN_BYTE;
                                load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, se_pw_2_w_brams[bram_indx], row_start_to_write + row);
                            }
                        }
                    }
                    #pragma omp atomic write
                    bram_load_done = 1;
                }
            }
            int acc_row = row_ofm;
            se_pw_store(se_pw_pe_2_arr, SE_PW_2_ACC_BRAM, acc_row * 4);
            
            ping_state = 1 - ping_state;
            pong_state = 1 - pong_state;
        }
        printf("[LOGS] Done SE Pointwise 2 Conv\n");
        print_bram_to_file("output/se_pw2.txt", SE_PW_2_ACC_BRAM, (SE_PW_2_COUT + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE);

        // ================================================== MUL ==================================================
        printf("[LOGS] Starting pointwise MUL\n");
        for(int se_out = 0; se_out < DW_C_OUT / BRAM_WIDTH_IN_BYTE; se_out++){

            for(int row_indx_in_16_channel = 0; row_indx_in_16_channel < DW_H_OUT * DW_W_OUT; row_indx_in_16_channel++){
                
                int expected_se_steps = (se_out + 1) * (BRAM_WIDTH_IN_BYTE / NUM_OF_SE_PE);


                int8_t se_row[16];
                for(int i = 0; i < 16; i++){
                    se_row[i] = (int8_t)SE_PW_2_ACC_BRAM[se_out][i];
                }
                // Get the DW out
                int8_t dw_out[16];
                int dw_data_idx = row_indx_in_16_channel * (DW_C_OUT / BRAM_WIDTH_IN_BYTE) + se_out;
                for(int i = 0; i < 16; i++){
                    dw_out[i] = (int8_t)DW_ACC_BRAM[dw_data_idx][i];
                }

                mul_act(&mul[0], se_row[0], dw_out[0]);
                mul_act(&mul[1], se_row[1], dw_out[1]);
                mul_act(&mul[2], se_row[2], dw_out[2]);
                mul_act(&mul[3], se_row[3], dw_out[3]);
                mul_act(&mul[4], se_row[4], dw_out[4]);
                mul_act(&mul[5], se_row[5], dw_out[5]);
                mul_act(&mul[6], se_row[6], dw_out[6]);
                mul_act(&mul[7], se_row[7], dw_out[7]);
                mul_act(&mul[8], se_row[8], dw_out[8]);
                mul_act(&mul[9], se_row[9], dw_out[9]);
                mul_act(&mul[10], se_row[10], dw_out[10]);
                mul_act(&mul[11], se_row[11], dw_out[11]);
                mul_act(&mul[12], se_row[12], dw_out[12]);
                mul_act(&mul[13], se_row[13], dw_out[13]);
                mul_act(&mul[14], se_row[14], dw_out[14]);
                mul_act(&mul[15], se_row[15], dw_out[15]);
                mul_store(MUL_BRAM, dw_data_idx);
            }

        }
        printf("[LOGS] Done MUL\n");
        print_bram_to_file("output/mul.txt", MUL_BRAM, DW_H_OUT * DW_W_OUT * DW_C_OUT / BRAM_WIDTH_IN_BYTE);

        // ========================================== Pointwise ==============================================
        printf("[LOGS] Starting the last PW ...\n");
        ping_state = READ;
        pong_state = WRITE;
        for(int tile = 0; tile < PW_LAST_COUT / NUM_OF_PE; tile++){ // Tính song song 16 kênh do đó chỉ cần tính C_OUT / PARALLEL lần.
            
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    for(int ho = 0; ho < PW_LAST_H; ho++){
                        for(int wo = 0; wo < PW_LAST_W; wo++){
                            pw_pe_array_reset_acc(pw_last_pe_arr);
                            int row_needed_for_one_pixel_depth = PW_LAST_CIN / BRAM_WIDTH_IN_BYTE; // Một vector 1x1xC_in.
                            int ifm_row_indx = (ho * PW_LAST_W + wo) * row_needed_for_one_pixel_depth;
                            
                            int w_row_indx = (ping_state == READ) ? ping_start_row : pong_start_row;
                            
                            // ================ Tinh va load =================
                            
                            for(int i = 0; i < PW_LAST_CIN / BRAM_WIDTH_IN_BYTE; i++){
                                int32_t *SE_OUT_32_BIT = MUL_BRAM[ifm_row_indx + i];
                                int8_t ifm[16];
                                for(int j = 0; j < 16; j++){
                                    ifm[j] = (int8_t)SE_OUT_32_BIT[j];
                                }

                                pw_pe_compute(&pw_last_pe_arr[0], &ifm, 0, PW_LAST_W0_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[1], &ifm, 0, PW_LAST_W1_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[2], &ifm, 0, PW_LAST_W2_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[3], &ifm, 0, PW_LAST_W3_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[4], &ifm, 0, PW_LAST_W4_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[5], &ifm, 0, PW_LAST_W5_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[6], &ifm, 0, PW_LAST_W6_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[7], &ifm, 0, PW_LAST_W7_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[8], &ifm, 0, PW_LAST_W8_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[9], &ifm, 0, PW_LAST_W9_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[10], &ifm, 0, PW_LAST_W10_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[11], &ifm, 0, PW_LAST_W11_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[12], &ifm, 0, PW_LAST_W12_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[13], &ifm, 0, PW_LAST_W13_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[14], &ifm, 0, PW_LAST_W14_BRAM, w_row_indx + i);
                                pw_pe_compute(&pw_last_pe_arr[15], &ifm, 0, PW_LAST_W15_BRAM, w_row_indx + i);
                            }
                            // ================================================
                            int row_needed_for_one_pixel_depth_output = PW_LAST_COUT / BRAM_WIDTH_IN_BYTE;
                            int acc_row = (ho * PW_LAST_W + wo) * row_needed_for_one_pixel_depth_output + tile; 
                            pw_pe_array_store(pw_last_pe_arr, PW_LAST_ACC_BRAM, acc_row);
                            
                        }
                    }
                }
                #pragma omp section
                { // Load Section
                    if(tile < (PW_LAST_COUT / NUM_OF_PE) - 1){
                            int w_row_indx_to_write = (ping_state == WRITE) ? ping_start_row : pong_start_row;
                            // Calculate start address for next tile
                        int start_addr_next_tile = pw_last_start_addr + (tile + 1) * NUM_OF_PE * PW_LAST_CIN;

                        for(int bram_indx = 0; bram_indx < NUM_OF_BRAM; bram_indx++){
                            for(int i = 0; i < PW_LAST_CIN / BRAM_WIDTH_IN_BYTE; i++){
                                int dram_addr = start_addr_next_tile + bram_indx * PW_LAST_CIN + i * BRAM_WIDTH_IN_BYTE;
                                load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, pw_last_w_brams[bram_indx], w_row_indx_to_write + i);
                            }
                        }
                    }
                }
            }
            // Hoan doi vai tro ping va pong
            ping_state = 1 - ping_state;
            pong_state = 1 - pong_state;
        }

        printf("[LOGS] Done the last PW\n");
        print_bram_to_file("output/pw_last_acc.txt", PW_LAST_ACC_BRAM, PW_LAST_H * PW_LAST_W * PW_LAST_COUT / BRAM_WIDTH_IN_BYTE);

        // ================================== Add ====================================
        int total_tile_per_chuck = DW_H_OUT * DW_W_OUT;
        int num_of_chunk = PW_LAST_COUT / BRAM_WIDTH_IN_BYTE;

        if (DW_STRIDE == 1 && PW_C_IN == PW_LAST_COUT) {
            printf("[LOGS] Starting add op (Residual)....\n");
            for(int tile = 0; tile < total_tile_per_chuck * num_of_chunk; tile++){
                add_reset();
                int8_t *input = PWCONV_IFM_BRAM[tile];
                
                int32_t *temp = PW_LAST_ACC_BRAM[tile];
                int8_t output[16];
                for(int i = 0; i < 16; i++){
                    output[i] = (int8_t)temp[i];
                }

                add_compute(&add_arr[0], input[0], output[0]);
                add_compute(&add_arr[1], input[1], output[1]);
                add_compute(&add_arr[2], input[2], output[2]);
                add_compute(&add_arr[3], input[3], output[3]);
                add_compute(&add_arr[4], input[4], output[4]);
                add_compute(&add_arr[5], input[5], output[5]);
                add_compute(&add_arr[6], input[6], output[6]);
                add_compute(&add_arr[7], input[7], output[7]);
                add_compute(&add_arr[8], input[8], output[8]);
                add_compute(&add_arr[9], input[9], output[9]);
                add_compute(&add_arr[10], input[10], output[10]);
                add_compute(&add_arr[11], input[11], output[11]);
                add_compute(&add_arr[12], input[12], output[12]);
                add_compute(&add_arr[13], input[13], output[13]);
                add_compute(&add_arr[14], input[14], output[14]);
                add_compute(&add_arr[15], input[15], output[15]);

                add_store(tile);
            }
            printf("[LOGS] Done add\n");
        } else {
            printf("[LOGS] Skipping Residual Add (Stride > 1 or Channel Mismatch)\n");
            for(int tile = 0; tile < total_tile_per_chuck * num_of_chunk; tile++){
                for(int i = 0; i < 16; i++){
                    OUTPUT[tile][i] = PW_LAST_ACC_BRAM[tile][i];
                }
            }
        }

    printf("[LOGS] ============ Done. =============\n");
    // print_bram_32_bit(DW_ACC_BRAM);



    // print_bram(GAP_BRAM);
    // print_bram_32_bit(SE_PW_1_ACC_BRAM);
    // print_bram_32_bit(SE_PW_2_ACC_BRAM);
    // print_bram_32_bit(MUL_BRAM);
    // print_bram_32_bit(PW_LAST_ACC_BRAM);
    print_bram_to_file("output.txt", OUTPUT, PW_LAST_H * PW_LAST_W * PW_LAST_COUT / BRAM_WIDTH_IN_BYTE);
}