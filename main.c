#include <stdio.h>
#include "dram.h"
#include "bram.h"
#include "PE.h"
void load_all_weights(int8_t *dram){
    int8_t (*w_brams[16])[16] = {
        W0_BRAM, W1_BRAM, W2_BRAM, W3_BRAM, W4_BRAM, W5_BRAM, W6_BRAM, W7_BRAM,
        W8_BRAM, W9_BRAM, W10_BRAM, W11_BRAM, W12_BRAM, W13_BRAM, W14_BRAM, W15_BRAM
    };
    int start_addr = 18816;
    
    // We have 384 Output Channels (Filters)
    // Each Filter has 96 Input Channels (depth)
    // 96 bytes / 16 bytes-per-row = 6 rows per filter
    int rows_per_filter = 6;
    int num_filters = 384; 

    for (int f = 0; f < num_filters; f++) {
        int bram_index = f % 16;
        int bram_row_offset = (f / 16) * rows_per_filter;
        
        for (int r = 0; r < rows_per_filter; r++) {
             // Calculate DRAM address assuming Filters are stored sequentially
             int dram_offset = start_addr + (f * rows_per_filter + r) * 16;
             load_bram(dram, dram_offset, 16, w_brams[bram_index], bram_row_offset + r);
        }
    }
}

int main(){
    if(init_dram("test/dram.txt", DRAM) == SYS_OK){
        printf("[LOGS] Read file successfully\n");
    }
    print_dram(DRAM);
    
    // Load IFM_BRAM
    for(int i = 0; i < 1176; i++){
        load_bram(DRAM, i * 16, 16, IFM_BRAM, i);
    }
    // Write me a function to load all the weight the each bram here
    load_all_weights(DRAM);
    for(int i = 0; i < 16; i++){
        print_bram(w_brams[i]);
    }
}