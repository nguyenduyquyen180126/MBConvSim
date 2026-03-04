#include <stdio.h>
#include "dram.h"
#include "bram.h"
#include "PE.h"
#include <stdint.h>
#include "config.h"


int main(){
    // ============= Load to dram ==============
    if(init_dram("test/dram.txt", DRAM) == SYS_OK){
        printf("[LOGS] Read file successfully\n");
    }
    // print_dram(DRAM);
    
    // ============== Load IFM_BRAM and weight bram ================
    for(int i = 0; i < H_in * W_in * C_IN / BRAM_WIDTH_IN_BYTE; i++){ // i - số hàng BRAM
        load_bram(DRAM, i * BRAM_WIDTH_IN_BYTE, BRAM_WIDTH_IN_BYTE, IFM_BRAM, i);
    }
    uint16_t counter = 1;
    while(counter != 0){
        
        int bram_indx = __builtin_ctz(counter);// Load BRAM thứ mấy
        int num_of_filter_in_bram = NUM_OF_FILTER / NUM_OF_PE;
        int row_per_filter = FILTER_DEPTH / BRAM_WIDTH_IN_BYTE;
        for(int i = 0; i < num_of_filter_in_bram; i++){ // Cái filter thứ mấy trong BRAM để tính
            for(int row = 0; row < row_per_filter; row++){

                int filter_indx = (i * NUM_OF_PE + bram_indx);
                int row_indx = row * BRAM_WIDTH_IN_BYTE;
                int dram_offset = WEIGHT_START_ADDR + filter_indx * FILTER_SIZE + row_indx;

                int bram_base_row_addr = i * row_per_filter;
                
                load_bram(DRAM, dram_offset, BRAM_WIDTH_IN_BYTE, w_brams[bram_indx], bram_base_row_addr + row);
            }
        }
        
        counter = counter << 1;
    }
    // print_bram(IFM_BRAM);
    for(int i = 0; i < 16; i++){
        print_bram(w_brams[i]);
    }
    
    // Vòng for tính output
    // Tính theo chiều ngang
    // for(int co = 0; co < C_OUT / PARALLEL; co++){ // Tính song song 16 kênh do đó chỉ cần tính C_OUT / PARALLEL lần
    //     for(int ho = 0; ho < H_out; ho++){
    //         for(int wo = 0; wo < W_out; wo++){
    //             pe_array_reset_acc(pe_array);
    //             // C_IN / PARALLEL số lần load để tính 1 kênh = số hàng 1 weght = số hàng của 1 pixel theo độ sâu
    //             int ifm_row_start = ho * W_in * C_IN / PARALLEL + wo * C_IN / PARALLEL;
    //             int w_row_start = co * C_IN / PARALLEL;
    //             for(int i = 0; i < C_IN / PARALLEL; i++){
    //                 pe_array_load_all(pe_array, ifm_row_start + i, w_row_start + i);
    //                 pe_array_compute(pe_array);
    //                 if (i == C_IN / PARALLEL - 1 ) valid = 1 ;
    //                 else valid = 0;
    //             }
    //             int acc_row = ((ho * W_out + wo) * (C_OUT / PARALLEL)) + co;
    //             if (valid) 
    //             pe_array_store(pe_array, acc_row);
    //         }
    //     }
    // }
    // Refactor the code above
    for(int tile = 0; tile < NUM_OF_FILTER / NUM_OF_PE; tile++){ // Tính song song 16 kênh do đó chỉ cần tính C_OUT / PARALLEL lần
        for(int ho = 0; ho < H_out; ho++){
            for(int wo = 0; wo < W_out; wo++){
                pe_array_reset_acc(pe_array);
                
                int row_needed_for_one_pixel_depth = C_IN / BRAM_WIDTH_IN_BYTE; // Một vector 1x1xC_in
                int ifm_row_indx = (ho * W_in + wo) * row_needed_for_one_pixel_depth;
                
                int row_need_for_one_filter = FILTER_DEPTH / BRAM_WIDTH_IN_BYTE;
                int w_row_indx = tile * row_need_for_one_filter;

                for(int i = 0; i < C_IN / PARALLEL; i++){
                    pe_compute(&pe_array[0], IFM_BRAM, ifm_row_indx + i, W0_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[1], IFM_BRAM, ifm_row_indx + i, W1_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[2], IFM_BRAM, ifm_row_indx + i, W2_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[3], IFM_BRAM, ifm_row_indx + i, W3_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[4], IFM_BRAM, ifm_row_indx + i, W4_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[5], IFM_BRAM, ifm_row_indx + i, W5_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[6], IFM_BRAM, ifm_row_indx + i, W6_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[7], IFM_BRAM, ifm_row_indx + i, W7_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[8], IFM_BRAM, ifm_row_indx + i, W8_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[9], IFM_BRAM, ifm_row_indx + i, W9_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[10], IFM_BRAM, ifm_row_indx + i, W10_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[11], IFM_BRAM, ifm_row_indx + i, W11_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[12], IFM_BRAM, ifm_row_indx + i, W12_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[13], IFM_BRAM, ifm_row_indx + i, W13_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[14], IFM_BRAM, ifm_row_indx + i, W14_BRAM, w_row_indx + i);
                    pe_compute(&pe_array[15], IFM_BRAM, ifm_row_indx + i, W15_BRAM, w_row_indx + i);
                }

                int row_needed_for_one_pixel_depth_output = C_OUT / BRAM_WIDTH_IN_BYTE;
                int acc_row = (ho * W_out + wo) * row_needed_for_one_pixel_depth_output + tile; //(Hàng bắt đầu + số hàng offset)
                
                pe_array_store(pe_array, acc_row);
            }
        }
    }
    print_bram_to_file("output/acc.txt", ACC_BRAM);
}