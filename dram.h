#ifndef DRAM_H
#define DRAM_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "config.h"

#define DRAM_SIZE 1000000

/*
    Mảng 1 chiều lớn chứa cả IFM, W, B
    0-18815 IFM C->W->H
    18816-end W Cin->H->W->Cout
*/
int8_t DRAM[DRAM_SIZE];
int init_dram(const char *file_name, int8_t *dram){
    FILE *fptr = fopen(file_name, "r");
    if(fptr == NULL){
        return SYS_INVALID_ARG;
    }
    int temp = 0, i = 0;
    while(fscanf(fptr, "%d", &temp) == 1){
        dram[i] = (int8_t)temp;
        i++;
    }
    fclose(fptr);
    return SYS_OK;
}
/*
@brief Debug purpose function
*/
void print_dram(int8_t *dram){
    for(int i = 0; i < DRAM_SIZE; i++){
        printf("%"PRId8" ", dram[i]);
    }
    printf("\n");
}
// int main(){
//     get_data("ifm.txt", DRAM);
    
// }
#endif