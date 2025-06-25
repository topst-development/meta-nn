/**
 *  Openedges Enlight NPU Testapplication
 *
 *  Testapplication main function.
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "npu.h"
#include <sys/types.h>
#include "npu_api.h"
#include <stdint.h>
#include <unistd.h>

#include "chip_test.h"

#define DUMP_NAME   "npu_test.txt"

enum
{
    TEST_NONE         =  0x00,
    TEST_REG          =  0x01,
    TEST_MLX_FUNC     =  0x02,
    TEST_NPU_UNIT     =  0x04,   
    TEST_NPU_NETWORK  =  0x08,
    TEST_RESET        =  0x10,
    TEST_LOAD_MLX     =  0x20,
    TEST_ECC          =  0x40,
    TEST_MLX_WDT      =  0x80,
    TEST_ALL          =  0xFF,
};

void print_usage(void)
{
    printf(" usage : options\n"
        " -d : device name number:   default : 0\n"
        "    0 for /dev/npu0\n"
        "    1 for /dev/npu1\n"
        "\n"
        " -t : test item:            default : TEST ALL\n"
        "    0 : ALL TEST \n"
        "    1 : REGISTER TEST\n"
        "    2 : MLX INSTRUCTION TEST\n"
        "    3 : NPU UNIT FUNCTION\n"
        "    4 : NPU NETWORK\n"
        "    5 : RESET\n"
        "    6 : ECC\n"
        "    7 : MLX Watchdog timer\n"
        " -h : print this message\n"
        " -q : ECC in quick mode\n"
        "    0 : full ECC test \n"
        "    1 : simple ECC test\n"
    );
}


int main(int argc, char **argv)
{
    int opt;
    int i, ret, num_regs;
    int test_item = 0;
    int dev_num = 0;
    npu_t* npu;

    int test_result = NPU_TEST_FAIL;
    int quick_mode = 0;

    while (-1 != (opt = getopt(argc, argv, "q:t:d:h"))) {
        switch (opt) {
            case 'q': quick_mode= atoi(optarg);  break;
            case 'd': dev_num   = atoi(optarg);  break;
            case 't': test_item = atoi(optarg); break;
            case 'h': print_usage(); exit(0);   break; 
        } 
    }    
    printf("Chip test Application\n");

    printf("%d: quick_mode\n", quick_mode);

    npu = npu_open(dev_num);
    if (!npu)
    {
	    fprintf(stderr, "npu open failed\n");
	    exit(1);
    }

    if (test_item == 0)
        test_item  = TEST_ALL;
    else if (test_item == 1)
        test_item  = TEST_REG;
    else if (test_item == 2)
        test_item  = TEST_MLX_FUNC;
    else if (test_item == 3)
        test_item  = TEST_NPU_UNIT;
    else if (test_item == 4)
        test_item  = TEST_NPU_NETWORK;
    else if (test_item == 5)
        test_item  = TEST_RESET;
    else if (test_item == 6)
        test_item  = TEST_ECC;
    else if (test_item == 7)
        test_item  = TEST_MLX_WDT;
    else {
	    fprintf(stderr, "wrong param -t: %d", test_item);
	    exit(1);
    }

    do {
        if (test_item & TEST_REG) {
            if (npu_reg_test_main(npu))
                break;
        }

        if (test_item & TEST_MLX_FUNC) {
            if (npu_mlx_test_main(npu))
                break;
        }

        if (test_item & TEST_NPU_UNIT) {
            if (test_npu_unit(npu))
                break;
        }   

        if (test_item & TEST_NPU_NETWORK) {
            if (test_npu_network(npu))
                break;
        }

        if (test_item & TEST_RESET) {
            if (test_npu_reset(npu))
                break;
        }

        if (test_item & TEST_ECC) {
            if (npu_ecc_test_main(npu, quick_mode))
                break;
        }

        if (test_item & TEST_MLX_WDT) {
            if (npu_wdt_test_main(npu))
                break;
        }
        
        test_result = NPU_TEST_SUCCESS;

    } while(0);

    if (npu)
        npu_close(npu);

    if (test_result == NPU_TEST_FAIL) {
        printf("chip test: Fail!\n");
        return 1;
    }

    printf("chip test: Success!\n");
    return 0;
}
