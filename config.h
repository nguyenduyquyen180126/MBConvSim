#ifndef CONFIG_H
#define CONFIG_H
/*
    function return status
*/
enum status_t{
    SYS_OK = 0,
    SYS_INVALID_ARG = -1,
    SYS_NOT_IMPLIMENT = -2,
    SYS_ERROR = -3
};
#define PW_C_IN 96
#define PW_C_OUT 384
#define PARALLEL 16
#define NUM_OF_PE 16
#define PW_H_in 14
#define PW_W_in 14
#define PW_H_out 14
#define PW_W_out 14
#define PW_FILTER_SIZE 96
#define PW_FILTER_DEPTH 96
#define PW_NUM_OF_FILTER 384
#define DMA_BW 128
#define BRAM_WIDTH_IN_BIT 128
#define BRAM_WIDTH_IN_BYTE 16
#define PIXEL_DAT_SIZE 8
#define PW_WEIGHT_START_ADDR PW_C_IN * PW_H_in * PW_W_in

#define DW_H_IN 14
#define DW_W_IN 14
#define DW_C_IN 384
#define DW_H_OUT 14
#define DW_W_OUT 14
#define DW_C_OUT 384
#define DW_H_K 3
#define DW_W_K 3
#define DW_C_K 1
#define DW_NUM_OF_K 384
#define DW_STRIDE 1

#endif