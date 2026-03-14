#include <stdio.h>
#include "dram.h"
#include "bram.h"
#include "PE.h"
#include <stdint.h>
#include "config.h"
#include <omp.h>
#include <unistd.h>


int main(){
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

    int dw_start_addr = 14 * 14 * 96 + 384 * 96;
    for(int i = 0; i < 3*3*384 / BRAM_WIDTH_IN_BYTE; i++){
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

    // =================== Tinh pipeline ===================
    printf("[LOGS] Starting PW-DW pipeline computation loops...\n");    
    int pw_row_compete = 0;

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            printf("[LOGS] Starting PW computation loops...\n");
            for(int tile = 0; tile < PW_NUM_OF_FILTER / NUM_OF_PE; tile++){ // Tính song song 16 kênh do đó chỉ cần tính C_OUT / PARALLEL lần.
                int pingpong_row_loaded = 0;
                for(int ho = 0; ho < PW_H_out; ho++){
                    for(int wo = 0; wo < PW_W_out; wo++){
                        pw_pe_array_reset_acc(pw_pe_array);
                        int row_needed_for_one_pixel_depth = PW_C_IN / BRAM_WIDTH_IN_BYTE; // Một vector 1x1xC_in.
                        int ifm_row_indx = (ho * PW_W_in + wo) * row_needed_for_one_pixel_depth;
                        
                        int w_row_indx = (ping_state == READ) ? ping_start_row : pong_start_row;
                        int w_row_indx_to_write = (ping_state == WRITE) ? ping_start_row : pong_start_row;

                        // ================ Tinh va load =================
                        
                        for(int i = 0; i < PW_FILTER_SIZE / NUM_OF_PE; i++){
                            // Song song viec load va tinh toan
                            #pragma omp parallel sections
                            {
                                #pragma omp section
                                {
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
                                #pragma omp section
                                {
                                    if(pingpong_row_loaded < PW_FILTER_SIZE * NUM_OF_BRAM / BRAM_WIDTH_IN_BYTE){
                                        int dram_start_addr = PW_WEIGHT_START_ADDR + (tile + 1) * PW_FILTER_SIZE * NUM_OF_PE + pingpong_row_loaded * 16;
                                        int bram_indx = pingpong_row_loaded / (PW_FILTER_SIZE / NUM_OF_PE);
                                        load_bram(DRAM, dram_start_addr, BRAM_WIDTH_IN_BYTE, pwconv_w_brams[bram_indx], w_row_indx_to_write + i);
                                        pingpong_row_loaded++;
                                    }
                                }
                            }
                        }
                        // ================================================
                        int row_needed_for_one_pixel_depth_output = PW_C_OUT / BRAM_WIDTH_IN_BYTE;
                        int acc_row = (ho * PW_W_out + wo) * row_needed_for_one_pixel_depth_output + tile; //(Hàng bắt đầu + số hàng offset)
                        pw_pe_array_store(pw_pe_array, acc_row);
                    }
                    pw_row_compete++;
                }
                // Hoan doi vai tro ping va pong
                ping_state = 1 - ping_state;
                pong_state = 1 - pong_state;
            }
            printf("[LOGS] PWConv Done.\n");
            fflush(stdout);

            print_bram_to_file("output/acc.txt", PWCONV_ACC_BRAM, 14 * 14 * 384 / 16);
        }
        #pragma omp section
        {
            printf("[LOGS] Starting DW computation loops...\n");
            for(int tile = 0; tile < DW_C_OUT / NUM_OF_PE; tile++){
                for(int ho = 0; ho < DW_H_OUT; ho++){
                    
                    int max_needed_row = ho * DW_STRIDE + (DW_H_K - 1) - pad_top;
                    if(max_needed_row >= PW_H_out) max_needed_row = PW_H_out - 1;
                    
                    int needed_compete_row = tile * PW_H_out + max_needed_row;
                    while(pw_row_compete <= needed_compete_row){
                        // usleep(1);
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
                    }
                }
            }
            printf("[LOGS] DONE DW loop\n");
        }
    }
    printf("[LOGS] ============ Done. =============\n");
    print_bram_32_bit(DW_ACC_BRAM);

    print_bram_to_file("output/acc.txt", PWCONV_ACC_BRAM, 14 * 14 * 384 / 16);
    print_bram_to_file("output/dw_acc.txt", DW_ACC_BRAM, 14 * 14 * 384 / 16);
}