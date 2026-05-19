#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/dram.h"
#include "include/bram.h"
#include "include/PE.h"
#include <stdint.h>
#include "include/config.h"
#include <omp.h>
#include <unistd.h>
#include "include/gap.h"
#include "include/mul.h"
#include "include/add.h"

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
    reset_performance_counters();
    ptr_cycles_load = &cycles_load_init;
    omp_set_max_active_levels(2);
    printf("====================================== Bat dau ======================================\n");
    // ===================== 1. Mô phỏng của lớp point wise conv =====================
    // ============= Load to dram =============
    if(init_dram("test/dram.txt", DRAM) == SYS_OK){
        printf("[LOGS] Read file successfully\n");
    } else {
        printf("[LOGS] Failed to read file\n");
        return -1;
    }
    
    // ============== Load IFM_BRAM ================
    printf("[LOGS] Load IFM BRAM\n");
    for(int i = 0; i < PW_H_in * PW_W_in * PW_C_IN / BRAM_WIDTH_IN_BYTE; i++){
        update_max(&max_row_ifm, i);
        load_bram(DRAM, i * BRAM_WIDTH_IN_BYTE, BRAM_WIDTH_IN_BYTE, PWCONV_IFM_BRAM, i);
    }
    printf("[LOGS] IFM BRAM Loaded.\n");
    
    // ============== Load PW weight ================
    printf("[LOGS] Loading PW Weight BRAMs...\n");
    uint16_t write_enable_weight = 1;
    while(write_enable_weight != 0){
        int bram_indx = __builtin_ctz(write_enable_weight);

        for(int i = 0; i < PW_C_IN / BRAM_WIDTH_IN_BYTE; i++){
            update_max(&max_row_pw_w_ping, i);
            load_bram(DRAM, PW_WEIGHT_START_ADDR + i * BRAM_WIDTH_IN_BYTE + bram_indx * PW_FILTER_SIZE, BRAM_WIDTH_IN_BYTE, pwconv_w_brams[bram_indx], i);
        }

        write_enable_weight = write_enable_weight << 1;
    }

    printf("[LOGS] PW Weight BRAMs Loaded.\n");
    // ================ Load DW weight ===============
    printf("[LOGS] Loading DW Weight BRAM...\n");

    int dw_start_addr = PW_H_in * PW_W_in * PW_C_IN + PW_NUM_OF_FILTER * PW_FILTER_SIZE;
    for(int i = 0; i < DW_H_K * DW_W_K * DW_NUM_OF_K / BRAM_WIDTH_IN_BYTE; i++){
        update_max(&max_row_dw_w, i);
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
    printf("[LOGS] Starting SE PW1 load weight...\n");
    int se_pw_1_start_addr = dw_start_addr + DW_H_K * DW_W_K * DW_NUM_OF_K;
    int se_pw_block_size = (SE_PW_1_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE;
    for(int bram_indx = 0; bram_indx < NUM_OF_SE_BRAM; bram_indx++){
        for(int row_indx = 0; row_indx < se_pw_block_size; row_indx++){
            update_max(&max_row_se1_w_ping, row_indx);
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
        for(int row_indx = 0; row_indx < se_pw_block_size; row_indx++){
            update_max(&max_row_se2_w_ping, row_indx);
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
            update_max(&max_row_pw_last_w_ping, i);
            int dram_addr = pw_last_start_addr + i * BRAM_WIDTH_IN_BYTE + bram_indx * PW_LAST_CIN;
            load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, pw_last_w_brams[bram_indx], ping_start_row + i);
        }
    }
    printf("[LOGS] PW Weight BRAMs Loaded.\n");

    // =================== Bien cho pipeline ===================
    printf("[LOGS] Starting PW-DW pipeline computation loops...\n");
    volatile int pw_row_compete = 0;
    volatile int dw_pixel_complete = 0;

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
            for(int tile = 0; tile < PW_NUM_OF_FILTER / NUM_OF_PE; tile++){
                while(compute_done == 0 || bram_load_done == 0){
                    usleep(1);
                }
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        compute_done = 0;
                        for(int ho = 0; ho < PW_H_out; ho++){
                            for(int wo = 0; wo < PW_W_out; wo++){
                                pw_pe_array_reset_acc(pw_pe_array);
                                int row_needed_for_one_pixel_depth = PW_C_IN / BRAM_WIDTH_IN_BYTE;
                                int ifm_row_indx = (ho * PW_W_in + wo) * row_needed_for_one_pixel_depth;
                                int w_row_indx = (ping_state == READ) ? ping_start_row : pong_start_row;
                                
                                for(int i = 0; i < PW_FILTER_SIZE / NUM_OF_PE; i++){
                                    #pragma omp atomic update
                                    cycles_pw1++;
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
                            
                                int row_needed_for_one_pixel_depth_output = PW_C_OUT / BRAM_WIDTH_IN_BYTE;
                                int acc_row = (ho * PW_W_out + wo) * row_needed_for_one_pixel_depth_output + tile;
                                pw_pe_array_store(pw_pe_array, PWCONV_ACC_BRAM, acc_row);
                            }
                            #pragma omp atomic update
                            pw_row_compete++;
                        }
                        compute_done = 1;
                    }
                    #pragma omp section
                    {
                        ptr_cycles_load = &cycles_load_pw1;
                        bram_load_done = 0;
                        if(tile < (PW_NUM_OF_FILTER / NUM_OF_PE) - 1){
                            int w_row_indx_to_write = (ping_state == WRITE) ? ping_start_row : pong_start_row;
                            int rows_per_bram = PW_C_IN / BRAM_WIDTH_IN_BYTE;
                            int next_tile_offset = (tile + 1) * NUM_OF_PE * PW_FILTER_SIZE;
                            
                            for(int bram_indx = 0; bram_indx < NUM_OF_BRAM; bram_indx++){
                                for(int r = 0; r < rows_per_bram; r++){
                                    if (w_row_indx_to_write == ping_start_row) update_max(&max_row_pw_w_ping, r);
                                    else update_max(&max_row_pw_w_pong, r);

                                    int dram_addr = PW_WEIGHT_START_ADDR + next_tile_offset + bram_indx * PW_FILTER_SIZE + r * BRAM_WIDTH_IN_BYTE;
                                    load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, pwconv_w_brams[bram_indx], w_row_indx_to_write + r);
                                }
                            }
                        }
                        bram_load_done = 1;
                    }
                }
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

                            int weight_row_addr = (k_i * DW_W_K + k_j) * (DW_NUM_OF_K / NUM_OF_PE) + tile;
                            int8_t *weight_row = DW_W_BRAM[weight_row_addr];

                            if(ifm_i_indx >= 0 && ifm_i_indx < DW_H_IN && ifm_j_indx >= 0 && ifm_j_indx < DW_W_IN){
                                int ifm_row_addr = (ifm_i_indx * DW_W_IN + ifm_j_indx) * (DW_C_IN / NUM_OF_PE) + tile;
                                int8_t *ifm_row = PWCONV_ACC_BRAM[ifm_row_addr];

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
                                #pragma omp atomic update
                                cycles_dw++;
                            } else {
                                int8_t ifm_row[16] = {0};
                                for(int j=0; j<16; j++) dw_pe_compute(&dw_pe_arr[j], ifm_row[j], weight_row[j]);
                                #pragma omp atomic update
                                cycles_dw++;
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
                    while(dw_pixel_complete <= i + tile * DW_H_OUT * DW_W_OUT) usleep(1);
                    int ifm_row_indx = i * (DW_C_OUT / NUM_OF_PE) + tile;
                    int8_t *ifm = DW_ACC_BRAM[ifm_row_indx];
                    for(int j=0; j<16; j++) gap_acc[j] += ifm[j];
                    #pragma omp atomic update
                    cycles_gap++;
                }
                for(int j=0; j<16; j++) gap_acc[j] /= DW_W_OUT * DW_H_OUT;
                update_max(&max_row_gap, tile);
                gap_acc_store(gap_acc, GAP_BRAM, tile);
            }
            print_bram_to_file("output/gap_acc.txt", GAP_BRAM, DW_C_OUT / BRAM_WIDTH_IN_BYTE);
            printf("[LOGS] Done global average pooling\n");
        }
    }
    // ====================================== SE PW 1 ======================================
    printf("[LOGS] Starting SE Pointwise 1 Conv\n");
    int ping_state = READ;
    int pong_state = WRITE;
    for(int chuck_ofm = 0; chuck_ofm < SE_PW_1_COUT / NUM_OF_SE_PE; chuck_ofm++){
        int row_start_to_read = (ping_state == READ) ? ping_start_row : pong_start_row;
        int row_start_to_write = (ping_state == WRITE) ? ping_start_row : pong_start_row;
        se_pw_reset(se_pw_pe_1_arr);
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                for(int row_ifm = 0; row_ifm < (SE_PW_1_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE; row_ifm++){
                    cycles_se1++;
                    pw_pe_compute(&se_pw_pe_1_arr[0], GAP_BRAM, row_ifm, SE_PW_1_W1_BRAM, row_start_to_read + row_ifm);
                    pw_pe_compute(&se_pw_pe_1_arr[1], GAP_BRAM, row_ifm, SE_PW_1_W2_BRAM, row_start_to_read + row_ifm);
                    pw_pe_compute(&se_pw_pe_1_arr[2], GAP_BRAM, row_ifm, SE_PW_1_W3_BRAM, row_start_to_read + row_ifm);
                    pw_pe_compute(&se_pw_pe_1_arr[3], GAP_BRAM, row_ifm, SE_PW_1_W4_BRAM, row_start_to_read + row_ifm);
                }
            }
            #pragma omp section
            {
                ptr_cycles_load = &cycles_load_se1;
                if(chuck_ofm + 1 < SE_PW_1_COUT / NUM_OF_SE_PE){
                    for(int bram_indx = 0; bram_indx < NUM_OF_SE_BRAM; bram_indx++){
                        for(int row = 0; row < (SE_PW_1_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE; row++){
                            if (row_start_to_write == ping_start_row) update_max(&max_row_se1_w_ping, row);
                            else update_max(&max_row_se1_w_pong, row);

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
                for(int row_ifm = 0; row_ifm < (SE_PW_2_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE; row_ifm++){
                    cycles_se2++;
                    int8_t *ifm_row = SE_PW_1_ACC_BRAM[row_ifm];
                    pw_pe_compute(&se_pw_pe_2_arr[0], (int8_t (*)[16])ifm_row, 0, SE_PW_2_W1_BRAM, row_start_to_read + row_ifm);
                    pw_pe_compute(&se_pw_pe_2_arr[1], (int8_t (*)[16])ifm_row, 0, SE_PW_2_W2_BRAM, row_start_to_read + row_ifm);
                    pw_pe_compute(&se_pw_pe_2_arr[2], (int8_t (*)[16])ifm_row, 0, SE_PW_2_W3_BRAM, row_start_to_read + row_ifm);
                    pw_pe_compute(&se_pw_pe_2_arr[3], (int8_t (*)[16])ifm_row, 0, SE_PW_2_W4_BRAM, row_start_to_read + row_ifm);
                }
            }
            #pragma omp section
            {
                ptr_cycles_load = &cycles_load_se2;
                if(row_ofm + 1 < SE_PW_2_COUT / NUM_OF_SE_PE){
                    for(int bram_indx = 0; bram_indx < NUM_OF_SE_BRAM; bram_indx++){
                        for(int row = 0; row < (SE_PW_2_CIN + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE; row++){
                            if (row_start_to_write == ping_start_row) update_max(&max_row_se2_w_ping, row);
                            else update_max(&max_row_se2_w_pong, row);

                            int dram_addr = se_pw_2_start_addr + (row_ofm + 1) * SE_PW_2_CIN * NUM_OF_SE_BRAM  + bram_indx * SE_PW_2_CIN + row * BRAM_WIDTH_IN_BYTE;
                            load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, se_pw_2_w_brams[bram_indx], row_start_to_write + row);
                        }
                    }
                }
            }
        }
        se_pw_store(se_pw_pe_2_arr, SE_PW_2_ACC_BRAM, row_ofm * 4);
        ping_state = 1 - ping_state;
        pong_state = 1 - pong_state;
    }
    printf("[LOGS] Done SE Pointwise 2 Conv\n");
    print_bram_to_file("output/se_pw2.txt", SE_PW_2_ACC_BRAM, (SE_PW_2_COUT + BRAM_WIDTH_IN_BYTE - 1) / BRAM_WIDTH_IN_BYTE);

    // ================================================== MUL ==================================================
    printf("[LOGS] Starting pointwise MUL\n");
    for(int se_out = 0; se_out < DW_C_OUT / BRAM_WIDTH_IN_BYTE; se_out++){
        for(int row_indx_in_16_channel = 0; row_indx_in_16_channel < DW_H_OUT * DW_W_OUT; row_indx_in_16_channel++){
            cycles_mul++;
            int8_t *se_row = SE_PW_2_ACC_BRAM[se_out];
            int dw_data_idx = row_indx_in_16_channel * (DW_C_OUT / BRAM_WIDTH_IN_BYTE) + se_out;
            int8_t *dw_out = DW_ACC_BRAM[dw_data_idx];
            for(int i=0; i<16; i++) mul_act(&mul[i], se_row[i], dw_out[i]);
            mul_store(MUL_BRAM, dw_data_idx);
        }
    }
    printf("[LOGS] Done MUL\n");
    print_bram_to_file("output/mul.txt", MUL_BRAM, DW_H_OUT * DW_W_OUT * DW_C_OUT / BRAM_WIDTH_IN_BYTE);

    // ========================================== Pointwise ==============================================
    printf("[LOGS] Starting the last PW ...\n");
    ping_state = READ;
    pong_state = WRITE;
    for(int tile = 0; tile < PW_LAST_COUT / NUM_OF_PE; tile++){
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                for(int ho = 0; ho < PW_LAST_H; ho++){
                    for(int wo = 0; wo < PW_LAST_W; wo++){
                        pw_pe_array_reset_acc(pw_last_pe_arr);
                        int row_needed_for_one_pixel_depth = PW_LAST_CIN / BRAM_WIDTH_IN_BYTE;
                        int ifm_row_indx = (ho * PW_LAST_W + wo) * row_needed_for_one_pixel_depth;
                        int w_row_indx = (ping_state == READ) ? ping_start_row : pong_start_row;
                        for(int i = 0; i < PW_LAST_CIN / BRAM_WIDTH_IN_BYTE; i++){
                            cycles_pw_last++;
                            int8_t *ifm = MUL_BRAM[ifm_row_indx + i];
                            pw_pe_compute(&pw_last_pe_arr[0], (int8_t (*)[16])ifm, 0, PW_LAST_W0_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[1], (int8_t (*)[16])ifm, 0, PW_LAST_W1_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[2], (int8_t (*)[16])ifm, 0, PW_LAST_W2_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[3], (int8_t (*)[16])ifm, 0, PW_LAST_W3_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[4], (int8_t (*)[16])ifm, 0, PW_LAST_W4_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[5], (int8_t (*)[16])ifm, 0, PW_LAST_W5_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[6], (int8_t (*)[16])ifm, 0, PW_LAST_W6_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[7], (int8_t (*)[16])ifm, 0, PW_LAST_W7_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[8], (int8_t (*)[16])ifm, 0, PW_LAST_W8_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[9], (int8_t (*)[16])ifm, 0, PW_LAST_W9_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[10], (int8_t (*)[16])ifm, 0, PW_LAST_W10_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[11], (int8_t (*)[16])ifm, 0, PW_LAST_W11_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[12], (int8_t (*)[16])ifm, 0, PW_LAST_W12_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[13], (int8_t (*)[16])ifm, 0, PW_LAST_W13_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[14], (int8_t (*)[16])ifm, 0, PW_LAST_W14_BRAM, w_row_indx + i);
                            pw_pe_compute(&pw_last_pe_arr[15], (int8_t (*)[16])ifm, 0, PW_LAST_W15_BRAM, w_row_indx + i);
                        }
                        int row_needed_for_one_pixel_depth_output = PW_LAST_COUT / BRAM_WIDTH_IN_BYTE;
                        int acc_row = (ho * PW_LAST_W + wo) * row_needed_for_one_pixel_depth_output + tile; 
                        pw_pe_array_store(pw_last_pe_arr, PW_LAST_ACC_BRAM, acc_row);
                    }
                }
            }
            #pragma omp section
            {
                ptr_cycles_load = &cycles_load_pw_last;
                if(tile < (PW_LAST_COUT / NUM_OF_PE) - 1){
                    int w_row_indx_to_write = (ping_state == WRITE) ? ping_start_row : pong_start_row;
                    int start_addr_next_tile = pw_last_start_addr + (tile + 1) * NUM_OF_PE * PW_LAST_CIN;
                    for(int bram_indx = 0; bram_indx < NUM_OF_BRAM; bram_indx++){
                        for(int i = 0; i < PW_LAST_CIN / BRAM_WIDTH_IN_BYTE; i++){
                            if (w_row_indx_to_write == ping_start_row) update_max(&max_row_pw_last_w_ping, i);
                            else update_max(&max_row_pw_last_w_pong, i);

                            int dram_addr = start_addr_next_tile + bram_indx * PW_LAST_CIN + i * BRAM_WIDTH_IN_BYTE;
                            load_bram(DRAM, dram_addr, BRAM_WIDTH_IN_BYTE, pw_last_w_brams[bram_indx], w_row_indx_to_write + i);
                        }
                    }
                }
            }
        }
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
            cycles_add++;
            add_reset();
            int8_t *input = PWCONV_IFM_BRAM[tile];
            int8_t *output = PW_LAST_ACC_BRAM[tile];
            for(int i = 0; i < 16; i++) add_compute(&add_arr[i], input[i], output[i]);
            add_store(tile);
        }
        printf("[LOGS] Done add\n");
    } else {
        printf("[LOGS] Skipping Residual Add (Stride > 1 or Channel Mismatch)\n");
        for(int tile = 0; tile < total_tile_per_chuck * num_of_chunk; tile++){
            update_max(&max_row_output, tile);
            for(int i = 0; i < 16; i++) OUTPUT[tile][i] = PW_LAST_ACC_BRAM[tile][i];
        }
    }
    printf("[LOGS] ============ Done. =============\n");
    report_bram_usage();
    report_performance();
    print_bram_to_file("output.txt", OUTPUT, PW_LAST_H * PW_LAST_W * PW_LAST_COUT / BRAM_WIDTH_IN_BYTE);
}