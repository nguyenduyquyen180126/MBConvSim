#include "dram.h"


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

void print_dram(int8_t *dram){
    for(int i = 0; i < DRAM_SIZE; i++){
        printf("%"PRId8" ", dram[i]);
    }
    printf("\n");
}