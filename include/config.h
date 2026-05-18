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

extern int PW_C_IN;
extern int PW_C_OUT;
extern int PARALLEL;
extern int NUM_OF_PE;
extern int NUM_OF_BRAM;
extern int PW_H_in;
extern int PW_W_in;
extern int PW_H_out;
extern int PW_W_out;
extern int PW_FILTER_SIZE;
extern int PW_FILTER_DEPTH;
extern int PW_NUM_OF_FILTER;
extern int DMA_BW;
extern int BRAM_WIDTH_IN_BIT;
extern int BRAM_WIDTH_IN_BYTE;
extern int PIXEL_DAT_SIZE;

extern int PW_WEIGHT_START_ADDR;
extern int DW_H_IN;
extern int DW_W_IN;
extern int DW_C_IN;
extern int DW_H_OUT;
extern int DW_W_OUT;
extern int DW_C_OUT;
extern int DW_H_K;
extern int DW_W_K;
extern int DW_C_K;
extern int DW_NUM_OF_K;
extern int DW_STRIDE;

extern int NUM_OF_SE_PE;
extern int NUM_OF_SE_BRAM;
extern int SE_PW_1_CIN;
extern int SE_PW_1_COUT;

extern int SE_PW_2_CIN;
extern int SE_PW_2_COUT;

extern int PW_LAST_CIN;
extern int PW_LAST_H;
extern int PW_LAST_W;
extern int PW_LAST_COUT;

#endif