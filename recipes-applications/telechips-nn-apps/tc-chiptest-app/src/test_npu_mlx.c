/**
 * Openedges Enlight NPU Test application
 *
 * NPU Verification MLX application
*/
#include <stdio.h>
#include <stdint.h>
#include "npu.h"
#include "npu_api.h"
#include "chip_test.h"

#include "npu_reg.h"
#include "test_npu.h"

enum {
    NPU_CTRL_RUN        = 0x1,
    NPU_CTRL_RESET      = 0x2,
    NPU_CTRL_CMD_SRC    = 0x4,
};

enum {
	NPU_IRQ_FULL_EMPTY  = 0x1,
	NPU_IRQ_HALF_EMPTY  = 0x2,
	NPU_IRQ_TRAP        = 0x4,
	NPU_IRQ_ALL         = 0x7,
};

static unsigned int npu_num_core = 0;
static unsigned int npu_major_ver = 0;
static unsigned int npu_minor_ver = 0;
static unsigned int npu_product_id = 0;
static unsigned int npu_ecc = 0;

struct mlx_inst_test{
    char kernel_name[256];
    int test_num;
};

#define NUM_MLX_TEST_KERNEL 7
static struct mlx_inst_test mlx_test[NUM_MLX_TEST_KERNEL] =
{
    { .kernel_name = "/lib/firmware/test_kernel_0.bin", .test_num = 5, },
    { .kernel_name = "/lib/firmware/test_kernel_1.bin", .test_num = 4, },
    { .kernel_name = "/lib/firmware/test_kernel_2.bin", .test_num = 2, },
    { .kernel_name = "/lib/firmware/test_kernel_3.bin", .test_num = 5, },
    { .kernel_name = "/lib/firmware/test_kernel_4.bin", .test_num = 4, },
    { .kernel_name = "/lib/firmware/test_kernel_5.bin", .test_num = 2, },
    { .kernel_name = "/lib/firmware/test_kernel_6.bin", .test_num = 1, },
};


static void wait_hci_busy(npu_t* npu)
{
    int busy = 0;
    int i;


    do {
        busy = 0;

        for (i = 0; i < npu_num_core; i++) {
            uint32_t data;
            npu_read_reg(npu, ADDR_NPU_MLX_C0_HCI_00 + (i * 0x40), &data);
            busy += data & 0x10;
        }

    } while(busy);
}

static void hci_run(npu_t* npu)
{
    uint32_t addr;

    for (int i = 0; i < npu_num_core; i = i + 1) {
        addr = ADDR_NPU_MLX_C0_HCI_00 + (i * 0x40);
        npu_write_reg(npu, addr, 0x101);
    }
}

int inst_test(npu_t* npu, int32_t num_test)
{
    int32_t ret;
    int32_t err_detect = 0;
    unsigned int data;
    unsigned int base;

    //clear terminate signal
    for(int i = 0; i < npu_num_core; i++){
        base = ADDR_NPU_MLX_C0_HCI_00 + 0x10 + (i * 0x40); 
        npu_write_reg(npu, base, 0x0);
    }
   
    for (int test_id=0; test_id < num_test; test_id++) {
        if (test_id == (num_test - 1)){
            //terminate signal
            for (int i = 0; i < npu_num_core; i++){
                base = ADDR_NPU_MLX_C0_HCI_00 + 0x10 + (i * 0x40); 
                npu_write_reg(npu, base, 0x1);
            }
        }

        npu_test_printf("instruction tests#%d \r\n", test_id);
        for (int i=0; i < npu_num_core; i++){
            base = ADDR_NPU_MLX_C0_HCI_00 + 0x4 + (i * 0x40); 
            npu_write_reg(npu, base, test_id);
            base = ADDR_NPU_MLX_C0_HCI_00 + 0x8 + (i * 0x40); 
            npu_write_reg(npu, base, 0xFFFFFFFF);
        }
        
        hci_run(npu);
        npu_test_sleep(1000);
        wait_hci_busy(npu);
        //_printf("Result of instruction test#%d \r\n", test_id);
        for (int i = 0; i < npu_num_core; i++){
            base = ADDR_NPU_MLX_C0_HCI_00 + 0x8 + (i * 0x40); 
            npu_read_reg(npu, base, &data);

            if (data != 0x0){
                npu_test_printf("TEST_INFO: FAIL\r\n");
                npu_test_printf("CORE#%d:  0x%08X\r\n", i, data);
            }
            err_detect |= data; //prevent overflow
        }

        if (err_detect) {
            npu_test_printf("Error Detected at test#%d\r\n", test_id);
            for (int i = 0; i < npu_num_core; i++){
                base = ADDR_NPU_MLX_C0_HCI_00 + 0x8 + (i * 0x40); 
                npu_read_reg(npu, base, &data);
                npu_test_printf("CORE#%d:  0x%08X\r\n", i, data);
            }
        }
    }

    ret = err_detect ? -1:0;  //-1: Fail, 0: Pass
    return ret;
}

int test_npu_mlx(npu_t* npu)
{
    int ret;

    for (int i = 0; i < NUM_MLX_TEST_KERNEL; i++) {
        npu_ecc_t ep;
        npu_chiptest_t ct;
        ep.disable_ue_fail = 0;
        ep.disable_ce_fail = 0;
        ep.disable_wdt = 0;

        ct.dev_mlx_bin_idx = i + 1;
        ct.dev_wdt_ext_cnt = 0;
        ct.dev_wdt_int_cnt = 0;
        ct.dev_ecc_test_ctrl = 0;
        ct.dev_mlx_err_inj_mask_data = 0;
        ct.dev_mlx_err_inj_mask_par = 0;

        ret = npu_reset(npu, 0, &ep, &ct);
        if (ret) {
            npu_test_printf("%d npu_reset() fail\n", ret);
            return ret;
        }

        ret = inst_test(npu, mlx_test[i].test_num);
        if (ret) {
            npu_test_printf("%d mlx kernel init_test[%d] fail\n", ret, i);
            return ret;
        }

        for (int i = 0; i < npu_num_core; i++) {
            uint32_t rdata;

            npu_read_reg(npu, ADDR_NPU_MLX_C0_ECC_CNT + (i * 0x40), &rdata);
            
            if (rdata) {
                npu_test_printf("non-zero ECC count\n");
                npu_test_printf("%d mlx kernel init_test[%d] fail\n", rdata, i);
                return ret;
            }
        }
    }

    return ret;
}

int npu_mlx_test_main(npu_t* npu)
{
    int err_cnt = 0;

    unsigned int val;

    npu_read_reg(npu, ADDR_NPU_ID_CODE, &val);

    npu_num_core   = (val >> 24) & 0xF;
    npu_major_ver  = (val >> 20) & 0xF;
    npu_minor_ver  = (val >> 18) & 0x3;
    npu_product_id = (val >>  0) & 0xFFFF;
    npu_ecc        = (val >> 16) & 0x1;
    npu_num_core   += 1;

    if (((val >> 16) & 0xFF) < 0x21) {
        npu_test_printf("Not Ndolphin CS\n");
        return 1;
    }

    if ((npu_product_id & 0xFFFF) != 0xED9E)
    {
        npu_test_printf("Unknown NPU prodcut ID\n");
        npu_test_printf("fail\n");
        npu_test_printf("========================\n\n");
        return 1;
    }

    npu_test_printf("========================\n");
    npu_test_printf("NPU product id: 0x%04X\n", npu_product_id);
    npu_test_printf("NPU major ver : 0x%X\n", npu_major_ver);
    npu_test_printf("NPU minor ver : 0x%X\n", npu_minor_ver);
    npu_test_printf("NPU ecc       : 0x%X\n", npu_ecc);
    npu_test_printf("NPU core num  : 0x%X\n", npu_num_core);
    npu_test_printf("========================\n");

    npu_test_printf("========================\n");
    npu_test_printf("MLX Instruction Test Start\n");
    npu_test_printf("========================\n");

    err_cnt = test_npu_mlx(npu);

    if (err_cnt)
    {
        npu_test_printf("fail\n");
        npu_test_printf("Total number of err: %d\r\n", err_cnt);
    }
    else
        npu_test_printf("success\n");

    npu_test_printf("========================\n");
    npu_test_printf("TEST_INFO: DONE\r\n");
    npu_test_printf("========================\n");

    // restore NPU for next test
    if (npu_reset(npu, 0, NULL, NULL)) {
        npu_test_printf("npu_reset fail\r\n");
    }

    return err_cnt;
}
