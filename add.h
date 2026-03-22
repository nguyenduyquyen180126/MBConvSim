#ifndef ADD_H
#define ADD_H
#include <stdint.h>
#include <inttypes.h>
int32_t OUTPUT[8000][16];
struct ADD{
    int8_t a;
    int8_t b;
    int32_t c;
};
struct ADD add_arr[16];
void add_compute(struct ADD *add, int8_t a, int8_t b){
    add->a = a;
    add->b = b;
    add->c = add->a + add->b;
}
void add_reset(){
    for(int i = 0; i < 16; i++){
        add_arr[i].c = 0;
    }
}
void add_store(int row_addr){
    for(int i = 0; i < 16; i++){
        OUTPUT[row_addr][i] = add_arr[i].c;
    }
}
#endif