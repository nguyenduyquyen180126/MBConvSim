#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
int8_t DRAM[1000];// Start with 0x0
int get_data(const char *file_name, int8_t *dram){
    FILE *fptr = fopen(file_name, "r");
    if(fptr == NULL){
        return 1;
    }
    int temp = 0;
    while(fscanf(fptr, "%" SCNd8, &temp) == 1){
        printf("%" PRId8"\n", temp);
    }
    fclose(fptr);
}
// int main(){
//     get_data("ifm.txt", DRAM);
    
// }