#ifndef LAYER_H
#define LAYER_H
#include <stdio.h>
// #include "config.h"
#include "gap.h"
#include "bram.h"
#include "dram.h"
#include "PE.h"
#include <stdint.h>
#include <inttypes.h>

void pointwise_conv(){
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
#endif