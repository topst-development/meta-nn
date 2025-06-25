/**
 * Openedges Enlight NPU Test application
 *
 * NPU Verification register test application
*/
#include <stdio.h>
#include "npu_reg.h"
#include "test_npu.h"
#include "npu_api.h"
#include "chip_test.h"

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

enum {
    NPU_OPCODE_NOP      = 0x0,
    NPU_OPCODE_RUN      = 0x1,
    NPU_OPCODE_WR_REG   = 0x2,
    NPU_OPCODE_TRAP     = 0x3,
};


#define HCI_REG_SIZE                            (0x40)

#define OFF_NPU_MLX_CO_ECC_CTRL                 (0x20)
#define OFF_NPU_MLX_CO_ECC_ERR_CNT              (0x24)
#define OFF_NPU_MLX_CO_ECC_CE_ADDR              (0x28)
#define OFF_NPU_MLX_CO_ECC_CE_DATA              (0x2C)
#define OFF_NPU_MLX_CO_ECC_UE_ADDR              (0x30)
#define OFF_NPU_MLX_CO_ECC_UE_DATA              (0x34)
#define OFF_NPU_MLX_CO_ECC_ERR_INJ_MASK_DATA    (0x38)
#define OFF_NPU_MLX_CO_ECC_ERR_INJ_MASK_PAR     (0x3C)

#define MAX_NUM_CORE 4
static unsigned int npu_num_core = 0;
static unsigned int npu_major_ver = 0;
static unsigned int npu_minor_ver = 0;
static unsigned int npu_product_id = 0;
static unsigned int npu_ecc = 0;
static unsigned int s_quick_mode = 0;

static int npu_ecc_cmd_buf_err_inject_test(
    npu_t* npu,
    int err_type);
static int npu_ecc_gbuf_err_inject_test(
    npu_t* npu,
    int err_type);
static int npu_mlx_ecc_sram_err_inject_test(
    npu_t* npu,
    unsigned int  err_data,
    unsigned int err_pari);
static int npu_mlx_ecc_sram_err_inject_test_core(
    npu_t* npu,
    unsigned int core_id,
    unsigned int expect_ce_addr,
    unsigned int expect_ue_addr,
    unsigned int expect_ce_cnt,
    unsigned int expect_ue_cnt,
    unsigned int expect_ce_status,
    unsigned int expect_ue_status,
    unsigned int expect_irq_reason);

static int run_network(npu_t* npu, char* c, char* w, char* s, char* i, char* o);

int test_npu_ecc_cmd_buf(npu_t* npu)
{
    int test_result;
    int ebit0, ebit1, err_type;
    unsigned int max_err_bit_pos0 = 137; //1 ~ 128 + 9(parity)
    unsigned int max_err_bit_pos1 = 137;

    const int no_err_test = 1;
    const int ce_err_test = 1;
    const int ue_err_test = 1;

    npu_ecc_t ecc_para;
    npu_chiptest_t ct_para;

    if (no_err_test) {
        npu_test_printf("CBUF: No err test\r\n");

        unsigned int data;
        // No err test
        ebit0 = 0;
        ebit1 = 0;

        ecc_para.disable_ue_fail = 0;
        ecc_para.disable_ce_fail = 0;
        ecc_para.disable_wdt = 0;

        ct_para.dev_mlx_bin_idx = 0;
        ct_para.dev_wdt_ext_cnt = 0;
        ct_para.dev_wdt_int_cnt = 0;

        data = 0;
        data += (ebit1 & 0x1FF) << 18;
        data += (ebit0 & 0x1FF) << 8;
        data += (0xF & 0xF)   << 4;
        data += (0x1 & 0x1)   << 0;
        ct_para.dev_ecc_test_ctrl = data;

        ct_para.dev_mlx_err_inj_mask_data = 0;
        ct_para.dev_mlx_err_inj_mask_par = 0;

        if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
            npu_test_printf("npu_reset fail\r\n");
        }

        npu_test_printf("%8d:%8d=ebit0:ebit1\r", ebit0, ebit1);

        err_type = 0;
        test_result = npu_ecc_cmd_buf_err_inject_test(npu, err_type);
        if (test_result) {
            npu_test_printf("failed w/ %d\r", test_result);
            return 1;
        }
        npu_test_printf("\npassed.\r\n");
    }

    if (ce_err_test) {
        int ret = 0;    
        npu_test_printf("CBUF: CE test\r\n");

        // CE test
        err_type = 1;
        for (ebit0 = 1; ebit0 <= max_err_bit_pos0; ebit0++) {
            unsigned int data;
            ebit1 = 0;

            ecc_para.disable_ue_fail = 0;
            ecc_para.disable_ce_fail = 0xF;
            ecc_para.disable_wdt = 0;

            ct_para.dev_mlx_bin_idx = 0;
            ct_para.dev_wdt_ext_cnt = 0;
            ct_para.dev_wdt_int_cnt = 0;

            data = 0;
            data += (ebit1 & 0x1FF) << 18;
            data += (ebit0 & 0x1FF) << 8;
            data += (0 & 0xF)   << 4;
            data += (1 & 0x1)   << 0;
            ct_para.dev_ecc_test_ctrl = data;

            ct_para.dev_mlx_err_inj_mask_data = 0;
            ct_para.dev_mlx_err_inj_mask_par = 0;

            if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
                npu_test_printf("npu_reset fail\r\n");
            }

            npu_test_printf("%8d:%8d=ebit0:ebit1\r", ebit0, ebit1);
            test_result = npu_ecc_cmd_buf_err_inject_test(npu, err_type);
            if (test_result) {
                npu_test_printf("failed w/ %d\r\n", test_result);
                return 1;
            }
        }

        for (ebit1 = 1; ebit1 <= max_err_bit_pos1; ebit1++) {
            unsigned int data;

            ebit0 = 0;

            ecc_para.disable_ue_fail = 0;
            ecc_para.disable_ce_fail = 0xF;
            ecc_para.disable_wdt = 0;

            ct_para.dev_mlx_bin_idx = 0;
            ct_para.dev_wdt_ext_cnt = 0;
            ct_para.dev_wdt_int_cnt = 0;

            data = 0;
            data += (ebit1 & 0x1FF) << 18;
            data += (ebit0 & 0x1FF) << 8;
            data += (0 & 0xF)   << 4;
            data += (1 & 0x1)   << 0;
            ct_para.dev_ecc_test_ctrl = data;

            ct_para.dev_mlx_err_inj_mask_data = 0;
            ct_para.dev_mlx_err_inj_mask_par = 0;

            if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
                npu_test_printf("npu_reset fail\r\n");
            }

            npu_test_printf("%8d:%8d=ebit0:ebit1\r", ebit0, ebit1);
            test_result = npu_ecc_cmd_buf_err_inject_test(npu, err_type);
            if (test_result) {
                npu_test_printf("failed w/ %d\r\n", test_result);
                return 1;
            }
        }
        npu_test_printf("\npassed.\r\n");
    }

    if (ue_err_test) {
        unsigned int data;

        /* CMD UE test makes NPU hangup */
        npu_test_printf("CBUF: UE test\r\n");
        // UE test
        err_type = 2;

        ebit0 = 1;
        ebit1 = 2;
        npu_test_printf("    %d:    %d=ebit0:ebit1\r\n", ebit0, ebit1);

        ecc_para.disable_ue_fail = 0xF;
        ecc_para.disable_ce_fail = 0;
        ecc_para.disable_wdt = 0;

        ct_para.dev_mlx_bin_idx = 0;
        ct_para.dev_wdt_ext_cnt = 0;
        ct_para.dev_wdt_int_cnt = 0;
        data = 0;
        //data += (ebit1 & 0x1FF) << 18;
        //data += (ebit0 & 0x1FF) << 8;
        //data += (0 & 0xF)   << 4;
        //data += (1 & 0x1)   << 0;
        ct_para.dev_ecc_test_ctrl = data;
        ct_para.dev_mlx_err_inj_mask_data = 0;
        ct_para.dev_mlx_err_inj_mask_par = 0;

        if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
            npu_test_printf("npu_reset fail\r\n");
        }

        // UE error injection after npu_reset()
        //  in order to run NPU DMA with error.
        data = 0;
        data += (ebit1 & 0x1FF) << 18;
        data += (ebit0 & 0x1FF) << 8;
        data += (0 & 0xF)   << 4;
        data += (1 & 0x1)   << 0;
        npu_write_reg(npu, ADDR_NPU_ECC_TEST_CTRL, data);

        test_result = npu_ecc_cmd_buf_err_inject_test(npu, err_type);
        if (test_result) {
            npu_test_printf("failed w/ %d\r\n", test_result);
            return 1;
        }
        npu_test_printf("\npassed.\r\n");
    }

    return 0;
}

int test_npu_ecc_gbuf(npu_t* npu)
{
    int test_result;
    int ebit0, ebit1, err_type;
    unsigned int min_err_bit_pos0;
    unsigned int max_err_bit_pos0;
    unsigned int min_err_bit_pos1;
    unsigned int max_err_bit_pos1;
    unsigned int temp_end0, temp_sta0;
    unsigned int temp_end1, temp_sta1;

    const int no_err_test = 1;
    const int ce_err_test = 1;
    const int ue_err_test = 1;

    npu_ecc_t ecc_para;
    npu_chiptest_t ct_para;

    if (no_err_test) {
        unsigned int data;

        npu_test_printf("GBUF: No err test\r\n");

        // NO err test
        ebit0 = 0;
        ebit1 = 0;
        err_type = 0;

        ecc_para.disable_ue_fail = 0x0;
        ecc_para.disable_ce_fail = 0x0;
        ecc_para.disable_wdt = 0;

        ct_para.dev_mlx_bin_idx = 0;
        ct_para.dev_wdt_ext_cnt = 0;
        ct_para.dev_wdt_int_cnt = 0;

        data = 0;
        data += (ebit1 & 0x1FF) << 18;
        data += (ebit0 & 0x1FF) << 8;
        data += (0xF & 0xF)     << 4;
        data += (0x1 & 0x1)     << 0;
        ct_para.dev_ecc_test_ctrl = data;

        ct_para.dev_mlx_err_inj_mask_data = 0;
        ct_para.dev_mlx_err_inj_mask_par = 0;

        if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
            npu_test_printf("npu_reset() fail");
        }

        npu_test_printf("%8d:%8d=ebit0:ebit1\r", ebit0, ebit1);
        test_result = npu_ecc_gbuf_err_inject_test(npu, err_type);
        if (test_result) {
            npu_test_printf("failed. %d\r\n", test_result);
            return 1;
        }
        npu_test_printf("\npassed.\r\n");
    }

    if (ce_err_test) {
        unsigned int data;
        npu_test_printf("GBUF: CE test\r\n");

        // CE test
        min_err_bit_pos0 = 1;
        temp_end0 = 129;
        temp_sta0 = 257;
        max_err_bit_pos0 = 265;

        min_err_bit_pos1 = 129;
        temp_end1 = 257;
        temp_sta1 = 266;
        max_err_bit_pos1 = 274;
        err_type = 1;

        for (ebit0 = min_err_bit_pos0; ebit0 <= max_err_bit_pos0; ebit0++) {
            if (ebit0 == temp_end0)
                ebit0 = temp_sta0;
            for (ebit1 = min_err_bit_pos1; ebit1 <= max_err_bit_pos1; ebit1++) {
                if (ebit1 == temp_end1)
                    ebit1 = temp_sta1;

                ecc_para.disable_ue_fail = 0x0;
                ecc_para.disable_ce_fail = 0xF;
                ecc_para.disable_wdt = 0;

                ct_para.dev_mlx_bin_idx = 0;
                ct_para.dev_wdt_ext_cnt = 0;
                ct_para.dev_wdt_int_cnt = 0;
                data = 0;
                data += (ebit1 & 0x1FF) << 18;
                data += (ebit0 & 0x1FF) << 8;
                data += (0xF & 0xF)     << 4;
                data += (0x0 & 0x1)     << 0;
                ct_para.dev_ecc_test_ctrl = data;
                ct_para.dev_mlx_err_inj_mask_data = 0;
                ct_para.dev_mlx_err_inj_mask_par = 0;

                if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
                    npu_test_printf("npu_reset() fail");
                }

                npu_test_printf("%8d:%8d=ebit0:ebit1\r", ebit0, ebit1);

                test_result = npu_ecc_gbuf_err_inject_test(npu, err_type);
                if (test_result) {
                    npu_test_printf("failed. %d\r\n", test_result);
                    return 1;
                }

                if (s_quick_mode)
                    break;
            }
        }
        npu_test_printf("\npassed.\r\n");
    }

    if (ue_err_test) {
        unsigned int data;
        npu_test_printf("GBUF: UE test\r\n");

        // UE test
        min_err_bit_pos0 = 1;
        temp_end0 = 129;
        temp_sta0 = 257;
        max_err_bit_pos0 = 265;

        min_err_bit_pos1 = 1;
        temp_end1 = 129;
        temp_sta1 = 257;
        max_err_bit_pos1 = 265;
        err_type = 2;

        for (ebit0 = min_err_bit_pos0; ebit0 <= max_err_bit_pos0; ebit0++) {
            if (ebit0 == temp_end0)
                ebit0 = temp_sta0;
            for (ebit1 = min_err_bit_pos1; ebit1 <= max_err_bit_pos1; ebit1++) {
                if (ebit1 == temp_end1)
                    ebit1 = temp_sta1;

                //CE case
                if (ebit0 == ebit1)
                    continue;

                ecc_para.disable_ue_fail = 0xF;
                ecc_para.disable_ce_fail = 0x0;
                ecc_para.disable_wdt = 0;

                ct_para.dev_mlx_bin_idx = 0;
                ct_para.dev_wdt_ext_cnt = 0;
                ct_para.dev_wdt_int_cnt = 0;

                data = 0;
                data += (ebit1 & 0x1FF) << 18;
                data += (ebit0 & 0x1FF) << 8;
                data += (0xF & 0xF)     << 4;
                data += (0x0 & 0x1)     << 0;
                ct_para.dev_ecc_test_ctrl = data;

                ct_para.dev_mlx_err_inj_mask_data = 0;
                ct_para.dev_mlx_err_inj_mask_par = 0;

                if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
                    npu_test_printf("npu_reset() fail");
                }

                npu_test_printf("    %4d:%4d = ebit0:ebit1\r", ebit0, ebit1);
                test_result = npu_ecc_gbuf_err_inject_test(npu, err_type);
                if (test_result) {
                    npu_test_printf("failed. %d\r\n", test_result);
                    return 1;
                }

                if (s_quick_mode)
                    break;
            }
        }

        min_err_bit_pos0 = 129;
        temp_end0 = 257;
        temp_sta0 = 266;
        max_err_bit_pos0 = 274;

        min_err_bit_pos1 = 129;
        temp_end1 = 257;
        temp_sta1 = 266;
        max_err_bit_pos1 = 274;
        err_type = 2;
        for (ebit0 = min_err_bit_pos0; ebit0 <= max_err_bit_pos0; ebit0++) {
            if (ebit0 == temp_end0)
                ebit0 = temp_sta0;
            for (ebit1 = min_err_bit_pos1; ebit1 <= max_err_bit_pos1; ebit1++) {
                if (ebit1 == temp_end1)
                    ebit1 = temp_sta1;

                //CE case
                if (ebit0 == ebit1)
                    continue;

                ecc_para.disable_ue_fail = 0xF;
                ecc_para.disable_ce_fail = 0x0;
                ecc_para.disable_wdt = 0;

                ct_para.dev_mlx_bin_idx = 0;
                ct_para.dev_wdt_ext_cnt = 0;
                ct_para.dev_wdt_int_cnt = 0;
                data = 0;
                data += (ebit1 & 0x1FF) << 18;
                data += (ebit0 & 0x1FF) << 8;
                data += (0xF & 0xF)     << 4;
                data += (0x0 & 0x1)     << 0;
                ct_para.dev_ecc_test_ctrl = data;

                ct_para.dev_mlx_err_inj_mask_data = 0;
                ct_para.dev_mlx_err_inj_mask_par = 0;

                if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
                    npu_test_printf("npu_reset() fail");
                }

                npu_test_printf("    %4d:%4d = ebit0:ebit1\r", ebit0, ebit1);
                test_result = npu_ecc_gbuf_err_inject_test(npu, err_type);
                if (test_result) {
                    npu_test_printf("failed. %d\r\n", test_result);
                    return 1;
                }
            }
            if (s_quick_mode)
                break;
        }
    }

    npu_test_printf("\npassed.\r\n");
    return 0;
}

int test_npu_ecc_mlx_mem(npu_t* npu)
{
    const int data_bit_num = 32;
    const int parity_bit_num = 7;
    int i, ret;
    unsigned int mask_data;
    unsigned int mask_pari;

    const int no_err_test = 1;
    const int ce_err_test = 1;
    const int ue_err_test = 1;

    npu_ecc_t ecc_para;
    npu_chiptest_t ct_para;

    if (no_err_test) {
    
        npu_test_printf("MLX SRAM: No err test\r\n");
    
        mask_data = 0;
        mask_pari = 0;

        npu_test_printf("0x%08X:0x%08X = mask_data:mask_pari\r\n",
            mask_data, mask_pari);
    
        ecc_para.disable_ue_fail = 0;
        ecc_para.disable_ce_fail = 0;
        ecc_para.disable_wdt = 0;
    
        ct_para.dev_mlx_bin_idx = 0;
        ct_para.dev_wdt_ext_cnt = 0;
        ct_para.dev_wdt_int_cnt = 0;
        ct_para.dev_ecc_test_ctrl = 0;
        ct_para.dev_mlx_err_inj_mask_data = mask_data;
        ct_para.dev_mlx_err_inj_mask_par = mask_pari + (1<<16);
    
        if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
            npu_test_printf("npu_reset fail\r\n");
            return -1;
        }
    
        ret = npu_mlx_ecc_sram_err_inject_test(
                npu,
                mask_data,
                mask_pari);
        if (ret) {
            npu_test_printf("failed!\r\n");
            return -1;
        }

        npu_test_printf("\npassed\r\n");
    }


    if (ce_err_test) {
        npu_test_printf("MLX SRAM: CE test\r\n");
    
        for (i = 0; i < data_bit_num; i++) {
            mask_data = 1 << i;
            mask_pari = 0;
    
            npu_test_printf("0x%08X:0x%08X = mask_data:mask_pari\r\n",
                mask_data, mask_pari);
    
            ecc_para.disable_ue_fail = 0;
            ecc_para.disable_ce_fail = 0;
            ecc_para.disable_wdt = 0;
    
            ct_para.dev_mlx_bin_idx = 0;
            ct_para.dev_wdt_ext_cnt = 0;
            ct_para.dev_wdt_int_cnt = 0;
            ct_para.dev_ecc_test_ctrl = 0;
            ct_para.dev_mlx_err_inj_mask_data = mask_data;
            ct_para.dev_mlx_err_inj_mask_par  = mask_pari + (1<<16);
    
            if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
                npu_test_printf("npu_reset fail\r\n");
                return -1;
            }

            ret = npu_mlx_ecc_sram_err_inject_test(npu, mask_data, mask_pari);
            if (ret)
                return -1;
        }
    
        for (i = 0; i < parity_bit_num; i++) {
            mask_data = 0;
            mask_pari = 1 << i;
    
            npu_test_printf("0x%08X:0x%08X = mask_data:mask_pari\r\n",
                mask_data, mask_pari);

            ecc_para.disable_ue_fail = 0;
            ecc_para.disable_ce_fail = 0;
            ecc_para.disable_wdt = 0;
    
            ct_para.dev_mlx_bin_idx = 0;
            ct_para.dev_wdt_ext_cnt = 0;
            ct_para.dev_wdt_int_cnt = 0;
            ct_para.dev_ecc_test_ctrl = 0;
            ct_para.dev_mlx_err_inj_mask_data = mask_data;
            ct_para.dev_mlx_err_inj_mask_par  = mask_pari + (1<<16);
    
            if (npu_reset(npu, 0, &ecc_para, &ct_para)) {
                npu_test_printf("npu_reset fail\r\n");
                return -1;
            }

            ret = npu_mlx_ecc_sram_err_inject_test(npu, mask_data, mask_pari);
            if (ret)
                return -1;
        }

        npu_test_printf("\npassed\r\n");
    }
    
    if (ue_err_test) {
        npu_test_printf("MLX SRAM: UE test\r\n");

        for (i = 0; i < data_bit_num; i++) {
            mask_data = 1 << i;
            int k;
    
            for (k = 0; k < parity_bit_num; k++) {
                mask_pari = 1 << k;
    
                npu_test_printf("0x%08X:0x%08X = mask_data:mask_pari\r\n",
                    mask_data, mask_pari);

                ecc_para.disable_ue_fail = 0;
                ecc_para.disable_ce_fail = 0;
                ecc_para.disable_wdt = 0;
    
                ct_para.dev_mlx_bin_idx = 0;
                ct_para.dev_wdt_ext_cnt = 0;
                ct_para.dev_wdt_int_cnt = 0;
                ct_para.dev_ecc_test_ctrl = 0;
                ct_para.dev_mlx_err_inj_mask_data = mask_data;
                ct_para.dev_mlx_err_inj_mask_par  = mask_pari + (1<<16);
    
                if (!npu_reset(npu, 0, &ecc_para, &ct_para)) {
                    return -1;
                }
                else {
                    npu_test_printf("In UE, npu_reset() shoud be fail\r\n");
                }
    
                ret = npu_mlx_ecc_sram_err_inject_test(
                        npu,
                        mask_data,
                        mask_pari);
                if (ret)
                    return -1;

                if (s_quick_mode)
                    break;
            }
        }

        npu_test_printf("\npassed\r\n");
    }

    npu_test_printf("\r\n");
    npu_test_printf("passed\r\n");

    return 0;
}

int npu_ecc_test_main(npu_t* npu, int quick_mode)
{
    int status = 0;

    unsigned int val;

    s_quick_mode = 1;

    npu_read_reg(npu, ADDR_NPU_ID_CODE, &val);

    npu_num_core   = (val >> 24) & 0xF;
    npu_major_ver  = (val >> 20) & 0xF;
    npu_minor_ver  = (val >> 18) & 0x3;
    npu_product_id = (val >>  0) & 0xFFFF;
    npu_ecc        = (val >> 16) & 0x1;
    npu_num_core   += 1;

    if (((val >> 16) & 0xFF) < 0x21) {
        npu_test_printf("ECC not supported in 0x%08X\n", npu_product_id);
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

    do {

        npu_test_printf("========================\n");
        npu_test_printf("ECC Test Start\n");
        npu_test_printf("========================\n");

        status = test_npu_ecc_cmd_buf(npu);
        if (status)
            break;

        status = test_npu_ecc_gbuf(npu);
        if (status)
            break;

        status = test_npu_ecc_mlx_mem(npu);
        if (status)
            break;

    } while(0);

    if (status)
        npu_test_printf("fail\n");
    else
        npu_test_printf("success\n");

    // restore NPU for next test
    if (npu_reset(npu, 0, NULL, NULL)) {
        npu_test_printf("npu_reset fail\r\n");
    }

    npu_test_printf("========================\n");
    npu_test_printf("TEST_INFO: DONE\r\n");
    npu_test_printf("========================\n");

    return status;
}

static void print_ecc_status(npu_t* npu);

static int npu_ecc_cmd_buf_err_inject_test(npu_t* npu, int err_type)
{
    unsigned int i, data;
    unsigned int  cmd_buf_size;
    int timeout_cnt = 10000;

    unsigned int irq_reason = 0;
    int ret = 0;

    int ecc_cnt_expect, ue_cnt, ce_cnt;
    const int debug=0;
    
    npu_ecc_wdt_status_t npu_ecc_status;

    if (err_type == 2) //UE
    {
        //UE
        char *cmd_fn  = "npu_cmd.bin";
        char *wght_fn = "quantized_network.bin";
        char *so_fn   = "net.so";
        char *in_fn   = "input.ia.bin";
        char *out_dir = NULL;

        ret = run_network(npu, cmd_fn, wght_fn, so_fn, in_fn, out_dir);
        if (ret < 0) {
            return 1;
        }

        FILE *fp = fopen(cmd_fn, "rb");
        fseek(fp, 0, SEEK_END);
        int file_size = ftell(fp);
        fclose(fp);

        ecc_cnt_expect = (file_size / 4) + 32 + 6;
        // 32: CMD BUF size
        //  4: Async fifo
        // 2/1: Downsizer
        ecc_cnt_expect = (ecc_cnt_expect + 3) / 4;

        if (ecc_cnt_expect > 255)
            ecc_cnt_expect = 255;
    }
    else {
        char *cmd_fn  = "npu_cmd.bin";
        char *wght_fn = "quantized_network.bin";
        char *so_fn   = "net.so";
        char *in_fn   = "input.ia.bin";
        char *out_dir = "";

        ret = run_network(npu, cmd_fn, wght_fn, so_fn, in_fn, out_dir);
        if (ret < 0) {
            npu_test_printf("%d: run_network\r\n", ret);
            return -1;
        }

        FILE *fp = fopen(cmd_fn, "rb");
        fseek(fp, 0, SEEK_END);
        int file_size = ftell(fp);
        fclose(fp);

        ecc_cnt_expect = (file_size / 4) + 32 + 6;
        // 32: CMD BUF size
        //  4: Async fifo
        // 2/1: Downsizer
        ecc_cnt_expect = (ecc_cnt_expect + 3) / 4;

        if (ecc_cnt_expect > 255)
            ecc_cnt_expect = 255;
    }

    //print_ecc_status(npu);
    npu_read_ecc_status(npu, &npu_ecc_status);

    // check list
    // 0. IRQ_CMD_BUF_ECC_CE/UE value of ADDR_NPU_IRQ_REASON
    // 1. CMD_BUF_ECC_CE/UE_CNT value of ADDR_NPU_ECC_CBUF_ECC_CNT

    ce_cnt = npu_ecc_status.cbuf.ce_cnt;
    ue_cnt = npu_ecc_status.cbuf.ue_cnt;
    irq_reason = npu_ecc_status.irq_reason;

    if (err_type == 1)
    {
        // CE
        if (((irq_reason >> 20) & 0x3) != 2)
            ret = 2;

        if (ecc_cnt_expect != ce_cnt)
        {
            npu_test_printf("ecc_cnt != ecc_cnt_expect, %d != %d\r\n",
                ce_cnt, ecc_cnt_expect);
            ret = 3;
        }

        if (0 != ue_cnt)
            ret = 4;
    }
    else if (err_type == 2)
    {
        // UE
        if (((irq_reason >> 20) & 0x3) != 1)
            ret = 5;

        if (0 != ce_cnt)
            ret = 6;

        if (ue_cnt == 0)
            ret = 7;
        //if (ecc_cnt_expect != ue_cnt)
        //    ret = 6;
    }
    else
    {
            // No err
        if (((irq_reason >> 20) & 0x3) != 0)
            ret = 10;

        if (0 != ce_cnt)
            ret = 11;

        if (0 != ue_cnt)
            ret = 12;
    }


    if (ret) {
        npu_print_main_regs(npu);

        npu_test_printf("0x%08x: ecc_cnt_expect \r\n", ecc_cnt_expect);
        npu_test_printf("0x%08x: ce_cnt         \r\n", ce_cnt);
        npu_test_printf("0x%08x: ue_cnt         \r\n", ue_cnt);
    }

    return ret;
}

static int npu_ecc_gbuf_err_inject_test(npu_t *npu, int err_type)
{
    unsigned int i, data;
    unsigned int* cmd_buf;
    unsigned int  cmd_buf_size;
    int timeout_cnt = 1000000;

    unsigned int irq_reason = 0;
    int ret = 0;

    int ecc_cnt_expect[4];
    int ue_cnt[4];
    int ce_cnt[4];
    const int debug = 0;

    npu_ecc_wdt_status_t npu_ecc_status;

    char *cmd_fn  = NULL;
    char *wght_fn = NULL;
    char *so_fn   = NULL;
    char *in_fn   = NULL;
    char *out_dir = NULL;

    //SSDlite300
    ecc_cnt_expect[0] = 0xFFFFFFFF;
    ecc_cnt_expect[1] = 0xFFFFFFFF;
    ecc_cnt_expect[2] = 0;
    ecc_cnt_expect[3] = 0;

    if (err_type == 2) {
        cmd_fn  = "npu_cmd.bin";
        wght_fn = "quantized_network.bin";
        so_fn   = "net.so";
        in_fn   = "input.ia.bin";
        out_dir = NULL;
    }
    else {
        cmd_fn  = "npu_cmd.bin";
        wght_fn = "quantized_network.bin";
        so_fn   = "net.so";
        in_fn   = "input.ia.bin";
        out_dir = "";
    }

    ret = run_network(npu, cmd_fn, wght_fn, so_fn, in_fn, out_dir);
    if (ret < 0) {
        return 1;
    }

    npu_read_ecc_status(npu, &npu_ecc_status);

    irq_reason = npu_ecc_status.irq_reason;

    for (i = 0; i < MAX_NUM_CORE; i++) {
        ue_cnt[i] = npu_ecc_status.gbuf[i].ue_cnt;
        ce_cnt[i] = npu_ecc_status.gbuf[i].ce_cnt;
    }

    // CE
    if (err_type == 1) {
        if (((irq_reason >> 16) & 0xF) == 0)
            ret = 1;

        for (i = 0; i < npu_num_core; i++) {
            if (ecc_cnt_expect[i] != ce_cnt[i]) {
                npu_test_printf("CE %d: %d =ecc_cnt_expect:ce_cnt\r\n", ecc_cnt_expect[i], ce_cnt[i]);
                ret = 2;
            }

            if (0 != ue_cnt[i]) {
                npu_test_printf("CE %d: ue_cnt\r\n", ue_cnt[i]);
                ret = 3;
            }
        }
    }

    // UE
    else if (err_type == 2) {

        if (((irq_reason >> 12) & 0xF) == 0)
            ret = 4;

        for (i = 0; i < npu_num_core; i++) {
            if (0 != ce_cnt[i]) {
                ret = 5;
                break;
            }

            if (ecc_cnt_expect[i] != ue_cnt[i]) {
                npu_test_printf("--[%d]0x%08x: 0x%08x\r\n", i, ecc_cnt_expect[i], ue_cnt[i]);
                ret = 6;
                break;
            }
        }
    }
    // No err
    else {
        if (((irq_reason >> 12) & 0xFF) != 0)
            ret = 10;

        for (i = 0; i < npu_num_core; i++) {
            if ((ce_cnt[i] != 0) || (ue_cnt[i] != 0)) {
                ret = 11;
                break;
            }
        }
    }

    if (ret) {
        npu_print_main_regs(npu);
    }

    return ret;
}

static int npu_mlx_ecc_sram_err_inject_test(
    npu_t* npu,
    unsigned int  ebit_mask_data,
    unsigned int  ebit_mask_pari)
{
    unsigned int i, data, base, ret;

    unsigned int expect_ce_addr;
    unsigned int expect_ue_addr;
    unsigned int expect_ce_cnt;
    unsigned int expect_ue_cnt;
    unsigned int expect_ce_status;
    unsigned int expect_ue_status;
    unsigned int expect_irq_reason;
    unsigned int error_bit_cnt;

    unsigned int sram_read_base = 0x80000000; //SRAM base
    unsigned int sram_read_size = 16;

    const int data_bit_num = 32;
    const int parity_bit_num = 7;

    // make expect value from bit injection
    error_bit_cnt = 0;

    for (i = 0; i < data_bit_num; i++)
        if ((ebit_mask_data >> i) & 1)
            error_bit_cnt++;

    for (i = 0; i < parity_bit_num; i++)
        if ((ebit_mask_pari >> i) & 1)
            error_bit_cnt++;

    if (error_bit_cnt > 2) {
        npu_test_printf("Wrong MLX error inject bit\r\n");
        npu_test_printf("0x%08x:err_bit_mask_data\r\n", ebit_mask_data);
        npu_test_printf("0x%08x:err_bit_mask_pari\r\n", ebit_mask_pari);
        return 1;
    }
    else if (error_bit_cnt == 2) {
        expect_ce_addr = 0;
        expect_ue_addr = sram_read_base;
        expect_ce_cnt = 0;
        if (sram_read_size < 255)
            expect_ue_cnt = sram_read_size;
        else
            expect_ue_cnt = 255;
        expect_ce_status = 0;
        expect_ue_status = 1;
        expect_irq_reason = 1;
    }
    else if (error_bit_cnt == 1) {
        expect_ce_addr = sram_read_base;
        expect_ue_addr = 0;
        if (sram_read_size < 255)
            expect_ce_cnt = sram_read_size;
        else
            expect_ce_cnt = 255;
        expect_ue_cnt = 0;
        expect_ce_status = 1;
        expect_ue_status = 0;
        expect_irq_reason = 1;
    }
    else {
        expect_ce_addr = 0;
        expect_ue_addr = 0;
        expect_ce_cnt = 0;
        expect_ue_cnt = 0;
        expect_ce_status = 0;
        expect_ue_status = 0;
        expect_irq_reason = 1;
    }

    if (0) {
        npu_test_printf("0x%08x: expect_ce_addr\r\n",    expect_ce_addr);
        npu_test_printf("0x%08x: expect_ue_addr\r\n",    expect_ue_addr);
        npu_test_printf("0x%08x: expect_ce_cnt\r\n",     expect_ce_cnt);
        npu_test_printf("0x%08x: expect_ue_cnt\r\n",     expect_ue_cnt);
        npu_test_printf("0x%08x: expect_ce_status\r\n",  expect_ce_status);
        npu_test_printf("0x%08x: expect_ue_status\r\n",  expect_ue_status);
        npu_test_printf("0x%08x: expect_irq_reason\r\n", expect_irq_reason);
    }

    for ( i= 0; i < npu_num_core; i++) {
        ret = npu_mlx_ecc_sram_err_inject_test_core(
                npu,
                i,
                expect_ce_addr,
                expect_ue_addr,
                expect_ce_cnt,
                expect_ue_cnt,
                expect_ce_status,
                expect_ue_status,
                expect_irq_reason);

        if (ret) {
            //int j;
            //npu_print_main_regs(npu);
            //for (j= 0; j < npu_num_core; j++)
            //    npu_print_hci_regs(npu, j);
            //    npu_test_printf("failed. %d\r\n", ret);
            break;
        }
    }

    return ret;
}

static int npu_mlx_ecc_sram_err_inject_test_core(
    npu_t* npu,
    unsigned int core_id,
    unsigned int expect_ce_addr,
    unsigned int expect_ue_addr,
    unsigned int expect_ce_cnt,
    unsigned int expect_ue_cnt,
    unsigned int expect_ce_status,
    unsigned int expect_ue_status,
    unsigned int expect_irq_reason
    )
{
    unsigned int ue_cnt;
    unsigned int ce_cnt;
    unsigned int ue_addr;
    unsigned int ce_addr;
    unsigned int ue_status;
    unsigned int ce_status;

    npu_ecc_wdt_status_t npu_ecc_status;
	npu_ecc_sram_t* ecc_mlx_sram = &npu_ecc_status.sram[core_id];

    npu_read_ecc_status(npu, &npu_ecc_status);

    ue_status = ecc_mlx_sram->ue_status;
    ce_status = ecc_mlx_sram->ce_status;

    if (ue_status != expect_ue_status) {
        npu_test_printf("UE status error\r\n");
        return -1;
    }

    if (ce_status != expect_ce_status) {
        npu_test_printf("CE status error\r\n");
        return -1;
    }

    ue_cnt = ecc_mlx_sram->ue_cnt;
    ce_cnt = ecc_mlx_sram->ce_cnt;

    if ((!expect_ue_cnt) ^ (!ue_cnt)) {
        npu_test_printf("UE count error %d %d\r\n",
            expect_ue_cnt, ue_cnt);
        return -1;
    }

    if ((!expect_ce_cnt) ^ (!ce_cnt)) {
        npu_test_printf("CE count error %d %d\r\n",
            expect_ce_cnt, ce_cnt);
        return -1;
    }

    ue_addr = ecc_mlx_sram->ue_addr;
    ce_addr = ecc_mlx_sram->ce_addr;

    if (expect_ue_status) {
        if (ue_addr != expect_ue_addr) {
            npu_test_printf("[0x%08x] UE addr error\r\n", ue_addr);
            return -1;
        }
    }

    if (expect_ce_status) {
        if (ce_addr != expect_ce_addr) {
            npu_test_printf("[0x%08x] CE addr error\r\n", ce_addr);
            return -1;
        }
    }

    return 0;
}

static int run_network(
    npu_t*  npu,
    char*   cmd_fn,
    char*   wght_fn,
    char*   so_fn,
    char*   in_fn,
    char*   out_fn
)
{
    int ret, input_size, output_size;

    npu_net_t* net = NULL;
    npu_buf_t* in = NULL;
    npu_buf_t* out = NULL;

    net = network_load_from_file(npu, so_fn, cmd_fn, wght_fn);

    // load input
    input_size = network_get_input_size(net);

    in = buffer_alloc(npu, input_size);
    load_input(in, in_fn, input_size);

    // prepare output buffer
    output_size = network_get_output_size(net);
    out = buffer_alloc(npu, output_size);

    // run network
    do {
        if (network_run(net, in, out, TIMEOUT_IN_MS) < 0) {
            npu_test_printf("%s - inference failed\n", cmd_fn);
            ret = -1;
            break;
        }

        if (out_fn != NULL) {
            if (out_verify(out, net->methods, out_fn)) {
                npu_test_printf("verification fail: %s \n", cmd_fn);
                ret = -1;
                break;
            }
            else {
            }
        }

        ret = 0;
    } while(0);

    if (net) {
        network_close(net);
        net = NULL;
    }

    if (in) {
        buffer_close(in);
        in = NULL;
    }

    if (out) {
        buffer_close(out);
        out = NULL;
    }

    return ret;
}

static void print_ecc_status(npu_t* npu)
{
    int i;
    int num_core = MAX_NUM_CORE;
    npu_ecc_wdt_status_t ecc;
	npu_ecc_cbuf_t* cbuf;
	npu_ecc_gbuf_t* gbuf;
	npu_ecc_sram_t* sram;

    npu_read_ecc_status(npu, &ecc);

    npu_test_printf("0x%08x:reason\r\n", ecc.irq_reason);

	//CBuf
	cbuf = &ecc.cbuf;
	npu_test_printf("0x%08x:cbuf->ue_cnt\n", cbuf->ue_cnt);
	npu_test_printf("0x%08x:cbuf->ce_cnt\n", cbuf->ce_cnt);

	//GBuf
	for (i = 0; i < num_core; i++) {
		gbuf = &ecc.gbuf[i];
		npu_test_printf("[%d]0x%08x:gbuf->ue_cnt\n", i, gbuf->ue_cnt);
		npu_test_printf("[%d]0x%08x:gbuf->ce_cnt\n", i, gbuf->ce_cnt);
	}

	//MLX SRAM
	for (i = 0; i < num_core; i++) {
		sram = &ecc.sram[i];

		npu_test_printf("[%d]:0x%08x:sram->ue_cnt \n", i, sram->ue_cnt );
		npu_test_printf("[%d]:0x%08x:sram->ce_cnt \n", i, sram->ce_cnt );
		npu_test_printf("[%d]:0x%08x:sram->ce_addr\n", i, sram->ce_addr);
		npu_test_printf("[%d]:0x%08x:sram->ce_data\n", i, sram->ce_data);
		npu_test_printf("[%d]:0x%08x:sram->ue_addr\n", i, sram->ue_addr);
		npu_test_printf("[%d]:0x%08x:sram->ue_data\n", i, sram->ue_data);
	}
}
    
