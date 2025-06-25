#ifndef _TEST_NPU_H_
#define _TEST_NPU_H_

#include "npu_api.h"

// npu_test_util.c
void npu_test_sleep(int ms);

#ifdef BARE_METAL
    #include "print.h"
    #define npu_test_printf(...) _printf(__VA_ARGS__)
#else
#define npu_test_printf(...) printf(__VA_ARGS__)
#endif

#endif
