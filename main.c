#include <stdio.h>
#include "dram.h"
#include "bram.h"
#include "PE.h"
#include <stdint.h>


int main(){
    // ============= Load to dram ==============
    if(init_dram("test/dram.txt", DRAM) == SYS_OK){
        printf("[LOGS] Read file successfully\n");
    }
    // print_dram(DRAM);
    
    // ============== Load IFM_BRAM and weight bram ================
    for(int i = 0; i < H_in * W_in * C_IN / BRAM_WIDTH_IN_BYTE; i++){ // i - số hàng BRAM
        load_bram(DRAM, i * 16, 16, IFM_BRAM, i);
    }
    uint16_t counter = 1;
    while(counter != 0){
        int dem = __builtin_ctz(counter); // Đếm số bit không đầu tiên để biết load BRAM thứ mấy
        for(int i = 0; i < C_OUT / PARALLEL; i++){ // w thứ i thuộc cùng 1 BRAM
            for(int fake_row = 0; fake_row < C_IN / PARALLEL; fake_row++){// coi như load vào một cái BRAM mới
                int addr_dram = (H_in * W_in * C_IN) + ((i * NUM_OF_PE + dem) * C_IN) + (fake_row * 16);
                load_bram(DRAM, addr_dram, 16, w_brams[dem], i * C_IN / PARALLEL + fake_row);
            }
        }
        
        counter = counter << 1;
    }
    // print_bram(IFM_BRAM);
    // for(int i = 0; i < 16; i++){
    //     print_bram(w_brams[i]);
    // }
    
    // Vòng for tính output
    // Tính theo chiều ngang
    for(int co = 0; co < C_OUT / PARALLEL; co++){ // Tính song song 16 kênh do đó chỉ cần tính C_OUT / PARALLEL lần
        for(int ho = 0; ho < H_out; ho++){
            for(int wo = 0; wo < W_out; wo++){
                pe_array_reset_acc(pe_array);
                // C_IN / PARALLEL số lần load để tính 1 kênh = số hàng 1 weght = số hàng của 1 pixel theo độ sâu
                int ifm_row_start = ho * W_in * C_IN / PARALLEL + wo * C_IN / PARALLEL;
                int w_row_start = co * C_IN / PARALLEL;
                for(int i = 0; i < C_IN / PARALLEL; i++){
                    pe_array_load_all(pe_array, ifm_row_start + i, w_row_start + i);
                    pe_array_compute(pe_array);
                }
                int acc_row = ((ho * W_out + wo) * (C_OUT / PARALLEL)) + co;
                pe_array_store(pe_array, acc_row);
            }
        }
    }
    print_bram_to_file("output/acc.txt", ACC_BRAM);
}