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
#define C_IN 96
#define C_OUT 384
#define PARALLEL 16
#define H_in 14
#define W_in 14
#define H_out 14
#define W_out 14


#endif