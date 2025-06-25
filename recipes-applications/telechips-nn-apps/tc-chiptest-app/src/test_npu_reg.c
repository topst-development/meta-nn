/**
 * Openedges Enlight NPU Test application
 *
 * NPU Verification register test application
*/

#ifdef BARE_METAL
#else
#   include <stdio.h>
#   include <stdint.h>
#   include "npu.h"
#   include "chip_test.h"
#endif

#include "npu_reg.h"
#include "npu_api.h"

#include "test_npu.h"

static unsigned int npu_num_core = 0;
static unsigned int npu_major_ver = 0;
static unsigned int npu_minor_ver = 0;
static unsigned int npu_nd_cs = 0;
static unsigned int npu_product_id = 0;
static unsigned int npu_ecc = 0;

static int apb_read_check(npu_t* npu, uint32_t reg_addr, uint32_t expected_data)
{
    uint32_t r_data;
    int error_observed = 0;

    npu_read_reg(npu, reg_addr, &r_data);

    error_observed = (r_data != expected_data) ? 1:0;

    if (error_observed) {
        npu_test_printf("error detected\r\n");
        npu_test_printf("ADDR:0x%08x | RDATA:0x%08x| REF:0x%08x \r\n",
            reg_addr, r_data, expected_data);
    }

    return error_observed;
}

static int check_int_reg(npu_t* npu, uint32_t int_reg_addr, uint32_t expected_data)
{
    int error_observed = 0;

    npu_write_reg(npu, ADDR_NPU_READ_INT_REG, int_reg_addr);
    //npu_test_printf("--0x%08x\r\n", int_reg_addr);
    //npu_test_sleep(1000);
    error_observed = apb_read_check(npu, ADDR_NPU_INT_REG_RDATA, expected_data);

    return error_observed;
}

enum {
    NPU_CTRL_RUN        = 0x1,
    NPU_CTRL_RESET      = 0x2,
    NPU_CTRL_CMD_SRC    = 0x4,
};

enum {
    NPU_OPCODE_NOP      = 0x0,
    NPU_OPCODE_RUN      = 0x1,
    NPU_OPCODE_WR_REG   = 0x2,
    NPU_OPCODE_TRAP     = 0x3,
};

int test_main_reg(npu_t* npu)
{
    int error_observed = 0;
    int temp;
    int id_code;

    // Reset
    npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RESET);

    // Step 1-1. Fill ones
    npu_write_reg(npu, ADDR_NPU_STATUS             , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND        , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_ID_CODE            , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_IRQ_REASON         , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_IRQ_ENABLE         , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_IRQ_MASK           , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_IRQ_CLEAR          , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_COLOR_CONV_0       , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_COLOR_CONV_1       , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_COLOR_CONV_2       , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_COLOR_CONV_BIAS    , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_READ_INT_REG       , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_INT_REG_RDATA      , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_CMD_CNT            , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR0         , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR1         , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR2         , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR3         , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR4         , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR5         , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR6         , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR7         , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_PERF_DMA           , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_PERF_COMP          , 0xFFFFFFFF);
    if (npu_nd_cs)
        npu_write_reg(npu, ADDR_NPU_PERF_ALL       , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_PERF_AXI_CONF      , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_INT_RST_CTRL       , 0xFFFFFFFF);

    if (npu_nd_cs) {
        npu_write_reg(npu, ADDR_NPU_CG_CTRL        , 0xFFFFFFFF);
    }

    for (int i = 0; i < npu_num_core; i = i + 1) {
        npu_write_reg(npu, ADDR_NPU_PERF_CNT_MM   + (i * 16), 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_PERF_CNT_DW   + (i * 16), 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_PERF_CNT_MISC + (i * 16), 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_PERF_CNT_MLX  + (i * 16), 0xFFFFFFFF);
    }

    if (npu_nd_cs) {
        npu_write_reg(npu, ADDR_NPU_ECC_CTRL           , 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_ECC_TEST_CTRL      , 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_ECC_CBUF_ECC_CNT   , 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_ECC_CBUF_ADDR      , 0xFFFFFFFF);
    }

    if (npu_nd_cs) {
        for (int i = 0; i < npu_num_core; i = i + 1) {
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_UE_CNT_C0 +   (i * 4) , 0xFFFFFFFF);
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_CE_CNT_C0 +   (i * 4) , 0xFFFFFFFF);
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_UE_ADDR0_C0 + (i * 8) , 0xFFFFFFFF);
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_UE_ADDR1_C0 + (i * 8) , 0xFFFFFFFF);
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_CE_ADDR0_C0 + (i * 8) , 0xFFFFFFFF);
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_CE_ADDR1_C0 + (i * 8) , 0xFFFFFFFF);
        }
    }

    npu_test_printf("MAIN_REG(Ones)\r\n");
    // Step 1-2. Check values after filling ones
    error_observed += apb_read_check(npu, ADDR_NPU_CONTROL           , 0x0);  // Not written
    error_observed += apb_read_check(npu, ADDR_NPU_STATUS            , 0x1F);  // RO, Q room size
    error_observed += apb_read_check(npu, ADDR_NPU_APB_COMMAND       , 0xFFFFFFFF);

    if (npu_nd_cs)
        id_code = 0x0121ed9e;
    else
        id_code = 0x0320ed9e;
    error_observed += apb_read_check(npu, ADDR_NPU_ID_CODE           , id_code);

    error_observed += apb_read_check(npu, ADDR_NPU_IRQ_REASON        , 0x0);  // write: clear reason
    if (npu_nd_cs)
    {
        error_observed += apb_read_check(npu, ADDR_NPU_IRQ_ENABLE    , 0xFFFFFF);
        error_observed += apb_read_check(npu, ADDR_NPU_IRQ_MASK      , 0xFFFFFF);
    }
    else
    {
        error_observed += apb_read_check(npu, ADDR_NPU_IRQ_ENABLE    , 0xFF);
        error_observed += apb_read_check(npu, ADDR_NPU_IRQ_MASK      , 0xFF);
    }
    error_observed += apb_read_check(npu, ADDR_NPU_IRQ_CLEAR         , 0x0);  // WO
    error_observed += apb_read_check(npu, ADDR_NPU_COLOR_CONV_0      , 0x3FFFFFFF);  // [29:0]
    error_observed += apb_read_check(npu, ADDR_NPU_COLOR_CONV_1      , 0x3FFFFFFF);  // [29:0]
    error_observed += apb_read_check(npu, ADDR_NPU_COLOR_CONV_2      , 0x3FFFFFFF);  // [29:0]
    error_observed += apb_read_check(npu, ADDR_NPU_COLOR_CONV_BIAS   , 0x3FFFFFFF);  // [29:0]
    error_observed += apb_read_check(npu, ADDR_NPU_READ_INT_REG      , 0x1FFF);
    //error_observed += apb_read_check(npu, ADDR_NPU_INT_REG_RDATA     , 0x0);  // RO
    error_observed += apb_read_check(npu, ADDR_NPU_CMD_CNT           , 0x0);  // RO
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR0        , 0xFFFFFFFF);  
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR1        , 0xFFFFFFFF);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR2        , 0xFFFFFFFF);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR3        , 0xFFFFFFFF);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR4        , 0xFFFFFFFF);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR5        , 0xFFFFFFFF);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR6        , 0xFFFFFFFF);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR7        , 0xFFFFFFFF);
    error_observed += apb_read_check(npu, ADDR_NPU_PERF_DMA          , 0xFFFFFFFF);
    error_observed += apb_read_check(npu, ADDR_NPU_PERF_COMP         , 0xFFFFFFFF);
    if (npu_nd_cs)
        error_observed += apb_read_check(npu, ADDR_NPU_PERF_ALL      , 0xFFFFFFFF);
    error_observed += apb_read_check(npu, ADDR_NPU_PERF_AXI_CONF     , 0xFF);

    if (npu_nd_cs)
        temp = ((1 << npu_num_core) - 1) + (((1 << npu_num_core) - 1) << 8);
    else
        temp = ((1 << npu_num_core) - 1) + (((1 << npu_num_core) - 1) << 8) + (1 << 24);

    error_observed += apb_read_check(npu, ADDR_NPU_INT_RST_CTRL , temp);
    if (npu_nd_cs) {
        error_observed += apb_read_check(npu, ADDR_NPU_CG_CTRL      , 0x73);  // [6:4], [1:0]
    }

    for (int i = 0; i < npu_num_core; i = i + 1)
    {
        error_observed += apb_read_check(npu, ADDR_NPU_PERF_CNT_MM   + (i * 16) , 0xFFFFFFFF);
        error_observed += apb_read_check(npu, ADDR_NPU_PERF_CNT_DW   + (i * 16) , 0xFFFFFFFF);
        error_observed += apb_read_check(npu, ADDR_NPU_PERF_CNT_MISC + (i * 16) , 0xFFFFFFFF);
        error_observed += apb_read_check(npu, ADDR_NPU_PERF_CNT_MLX  + (i * 16) , 0xFFFFFFFF);
    }

    if (npu_nd_cs) {
        error_observed += apb_read_check(npu, ADDR_NPU_ECC_CTRL            , 0xF1);  // [7:4], [0]
        error_observed += apb_read_check(npu, ADDR_NPU_ECC_TEST_CTRL       , 0x07FDFF00);
        error_observed += apb_read_check(npu, ADDR_NPU_ECC_CBUF_ECC_CNT    , 0x0);
        error_observed += apb_read_check(npu, ADDR_NPU_ECC_CBUF_ADDR       , 0x0);
        for(int i=0; i<npu_num_core; i=i+1){
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_UE_CNT_C0   + (i * 4) , 0x0);  // RO
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_CE_CNT_C0   + (i * 4) , 0x0);  // RO
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_UE_ADDR0_C0 + (i * 8) , 0x0);  // RO
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_UE_ADDR1_C0 + (i * 8) , 0x0);  // RO
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_CE_ADDR0_C0 + (i * 8) , 0x0);  // RO
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_CE_ADDR1_C0 + (i * 8) , 0x0);  // RO
        }
    }

    // Reset and clear IRQ
    npu_write_reg(npu, ADDR_NPU_IRQ_ENABLE     , 0x0);
    npu_write_reg(npu, ADDR_NPU_IRQ_REASON     , 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_CONTROL        , NPU_CTRL_RESET);

    // Step 2-1. Fill zeros
    npu_write_reg(npu, ADDR_NPU_STATUS           , 0x0);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND      , 0x0);
    npu_write_reg(npu, ADDR_NPU_ID_CODE          , 0x0);
    npu_write_reg(npu, ADDR_NPU_IRQ_REASON       , 0x0);
    npu_write_reg(npu, ADDR_NPU_IRQ_ENABLE       , 0x0);
    npu_write_reg(npu, ADDR_NPU_IRQ_MASK         , 0x0);
    npu_write_reg(npu, ADDR_NPU_IRQ_CLEAR        , 0x0);
    npu_write_reg(npu, ADDR_NPU_COLOR_CONV_0     , 0x0);
    npu_write_reg(npu, ADDR_NPU_COLOR_CONV_1     , 0x0);
    npu_write_reg(npu, ADDR_NPU_COLOR_CONV_2     , 0x0);
    npu_write_reg(npu, ADDR_NPU_COLOR_CONV_BIAS  , 0x0);
    npu_write_reg(npu, ADDR_NPU_READ_INT_REG     , 0x0);
    npu_write_reg(npu, ADDR_NPU_INT_REG_RDATA    , 0x0);
    npu_write_reg(npu, ADDR_NPU_CMD_CNT          , 0x0);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR0       , 0x0);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR1       , 0x0);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR2       , 0x0);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR3       , 0x0);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR4       , 0x0);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR5       , 0x0);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR6       , 0x0);
    npu_write_reg(npu, ADDR_NPU_BASE_ADDR7       , 0x0);
    npu_write_reg(npu, ADDR_NPU_PERF_DMA         , 0x0);
    npu_write_reg(npu, ADDR_NPU_PERF_COMP        , 0x0);
    if (npu_nd_cs)
        npu_write_reg(npu, ADDR_NPU_PERF_ALL     , 0x0);
    npu_write_reg(npu, ADDR_NPU_PERF_AXI_CONF    , 0x0);
    npu_write_reg(npu, ADDR_NPU_INT_RST_CTRL     , 0x0);
    if (npu_nd_cs) {
        npu_write_reg(npu, ADDR_NPU_CG_CTRL      , 0x0);
    }

    for (int i = 0; i < npu_num_core; i = i + 1) {
        npu_write_reg(npu, ADDR_NPU_PERF_CNT_MM   + (i * 16) , 0x0);
        npu_write_reg(npu, ADDR_NPU_PERF_CNT_DW   + (i * 16) , 0x0);
        npu_write_reg(npu, ADDR_NPU_PERF_CNT_MISC + (i * 16) , 0x0);
        npu_write_reg(npu, ADDR_NPU_PERF_CNT_MLX  + (i * 16) , 0x0);
    }

    if (npu_nd_cs) {
        npu_write_reg(npu, ADDR_NPU_ECC_CTRL           , 0x0);
        npu_write_reg(npu, ADDR_NPU_ECC_TEST_CTRL      , 0x0);
        npu_write_reg(npu, ADDR_NPU_ECC_CBUF_ECC_CNT   , 0x0);
        npu_write_reg(npu, ADDR_NPU_ECC_CBUF_ADDR      , 0x0);

        for(int i = 0; i < npu_num_core; i = i + 1) {
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_UE_CNT_C0   + (i * 4)     , 0x0);
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_CE_CNT_C0   + (i * 4)     , 0x0);
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_UE_ADDR0_C0 + (i * 8) , 0x0);
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_UE_ADDR1_C0 + (i * 8) , 0x0);
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_CE_ADDR0_C0 + (i * 8) , 0x0);
            npu_write_reg(npu, ADDR_NPU_ECC_GBUF_CE_ADDR1_C0 + (i * 8) , 0x0);
        }
    }

    npu_test_printf("MAIN_REG(Zeros)\r\n");
    // Step 2-2. Check values after filling zeros
    error_observed += apb_read_check(npu, ADDR_NPU_CONTROL             , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_STATUS              , 0x1F);
    error_observed += apb_read_check(npu, ADDR_NPU_APB_COMMAND         , 0x0);

    if (npu_nd_cs)
        id_code = 0x0121ed9e;
    else
        id_code = 0x0320ed9e;
    error_observed += apb_read_check(npu, ADDR_NPU_ID_CODE             , id_code);
    error_observed += apb_read_check(npu, ADDR_NPU_IRQ_REASON          , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_IRQ_ENABLE          , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_IRQ_MASK            , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_IRQ_CLEAR           , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_COLOR_CONV_0        , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_COLOR_CONV_1        , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_COLOR_CONV_2        , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_COLOR_CONV_BIAS     , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_READ_INT_REG        , 0x0);
    //error_observed += apb_read_check(npu, ADDR_NPU_INT_REG_RDATA     , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_CMD_CNT             , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR0          , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR1          , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR2          , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR3          , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR4          , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR5          , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR6          , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_BASE_ADDR7          , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_PERF_DMA            , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_PERF_COMP           , 0x0);
    if (npu_nd_cs)
        error_observed += apb_read_check(npu, ADDR_NPU_PERF_ALL        , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_PERF_AXI_CONF       , 0x0);
    error_observed += apb_read_check(npu, ADDR_NPU_INT_RST_CTRL        , 0x0);
    if (npu_nd_cs)
        error_observed += apb_read_check(npu, ADDR_NPU_CG_CTRL         , 0x0);

    for (int i = 0; i < npu_num_core; i = i + 1){
        error_observed += apb_read_check(npu, ADDR_NPU_PERF_CNT_MM   + (i * 16), 0x0);
        error_observed += apb_read_check(npu, ADDR_NPU_PERF_CNT_DW   + (i * 16), 0x0);
        error_observed += apb_read_check(npu, ADDR_NPU_PERF_CNT_MISC + (i * 16), 0x0);
        error_observed += apb_read_check(npu, ADDR_NPU_PERF_CNT_MLX  + (i * 16), 0x0);
    }

    if (npu_nd_cs) {
        error_observed += apb_read_check(npu, ADDR_NPU_ECC_CTRL             , 0x0);
        error_observed += apb_read_check(npu, ADDR_NPU_ECC_TEST_CTRL        , 0x0);
        error_observed += apb_read_check(npu, ADDR_NPU_ECC_CBUF_ECC_CNT     , 0x0);
        error_observed += apb_read_check(npu, ADDR_NPU_ECC_CBUF_ADDR        , 0x0);

        for(int i = 0; i < npu_num_core; i = i + 1){
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_UE_CNT_C0   + (i * 4) , 0x0);
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_CE_CNT_C0   + (i * 4) , 0x0);
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_UE_ADDR0_C0 + (i * 8) , 0x0);
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_UE_ADDR1_C0 + (i * 8) , 0x0);
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_CE_ADDR0_C0 + (i * 8) , 0x0);
            error_observed += apb_read_check(npu, ADDR_NPU_ECC_GBUF_CE_ADDR1_C0 + (i * 8) , 0x0);
        }
    }

    return error_observed;
}

int test_hci(npu_t* npu)
{
    int i, j;
    int error_observed = 0;

    unsigned int hci_base;
    unsigned int addr;

    hci_base = ADDR_NPU_MLX_C0_HCI_00; 

    // Step 1-1. Fill ones
    for (i = 0; i < npu_num_core; i = i + 1) {
        int reg_num;

        if (npu_nd_cs)
            reg_num = 16;
        else
            reg_num = 8;

        for (j = 1; j < reg_num; j++) {
            addr = hci_base + (0x4 * j) + (i * 0x40);
            npu_write_reg(npu, addr, 0xFFFFFFFF);
        }
    }
    
    // 1-2. Check
    npu_test_printf("HCI_REG(Ones)\r\n");
    for (i = 0; i < npu_num_core; i = i + 1){
        // MLX control reg  test
        j = 0;
        addr = hci_base + (0x4 * j) + (i * 0x40);
        error_observed += apb_read_check(npu, addr, 0x0);

        // MLX param regs  test
        for (j = 1; j < 8; j++) {
            addr = hci_base + (0x4 * j) + (i * 0x40);
            error_observed += apb_read_check(npu, addr, 0xFFFFFFFF);
        }

        if (npu_nd_cs) {
            addr = hci_base + (0x4 * 8) + (i * 0x40);
            error_observed += apb_read_check(npu, addr, 0x00030001);
            addr = hci_base + (0x4 * 9) + (i * 0x40);
            error_observed += apb_read_check(npu, addr, 0x00FF00FF);
            addr = hci_base + (0x4 * 10) + (i * 0x40);
            error_observed += apb_read_check(npu, addr, 0x00000000);
            addr = hci_base + (0x4 * 11) + (i * 0x40);
            error_observed += apb_read_check(npu, addr, 0x00000000);
            addr = hci_base + (0x4 * 12) + (i * 0x40);
            error_observed += apb_read_check(npu, addr, 0x00000000);
            addr = hci_base + (0x4 * 13) + (i * 0x40);
            error_observed += apb_read_check(npu, addr, 0x00000000);
            addr = hci_base + (0x4 * 14) + (i * 0x40);
            error_observed += apb_read_check(npu, addr, 0xFFFFFFFF);
            addr = hci_base + (0x4 * 15) + (i * 0x40);
            error_observed += apb_read_check(npu, addr, 0x0001007F);
        }
    }

    // Step 2-1. zeros
    for (i = 0; i < npu_num_core; i = i + 1){
        int reg_num;

        if (npu_nd_cs)
            reg_num = 16;
        else
            reg_num = 8;

        for (j = 1; j < reg_num; j++) {
            addr = hci_base + (0x4 * j) + (i * 0x40);
            npu_write_reg(npu, addr, 0x0);
        }
    }
    
    npu_test_printf("HCI_REG(Zeros)\r\n");
    // 2-2. Check
    for (i = 0; i < npu_num_core; i = i + 1) {
        int reg_num;

        if (npu_nd_cs)
            reg_num = 16;
        else
            reg_num = 8;

        // MLX control reg  test
        j = 0;
        addr = hci_base + (0x4 * j) + (i * 0x40);
        error_observed += apb_read_check(npu, addr, 0x0);

        for (j = 1; j < reg_num; j++) {
            addr = hci_base + (0x4 * j) + (i * 0x40);
            error_observed += apb_read_check(npu, addr, 0x0);
        }
    }
    return error_observed;
}

int test_int_reg_id(npu_t* npu)
{
    uint64_t iaddr_core_idx;
    int error_observed = 0;
    
    for (int i = 0; i < npu_num_core; i = i + 1) {
        iaddr_core_idx = i<<11;
        // MM
        npu_test_printf("CORE%d MM_ID\r\n", i);
        error_observed += check_int_reg(npu, (uint32_t) (IADDR_MM|iaddr_core_idx), 0x10000000);
    
        // DW
        npu_test_printf("CORE%d DW_ID\r\n", i);
        error_observed += check_int_reg(npu, (uint32_t) (IADDR_DW|iaddr_core_idx), 0x20000000);
      
        // MISC
        npu_test_printf("CORE%d MISC_ID\r\n", i);
        error_observed += check_int_reg(npu, (uint32_t) (IADDR_MISC|iaddr_core_idx), 0x30000000);
        
        // MLX
        npu_test_printf("CORE%d MLX_ID\r\n", i);
        error_observed += check_int_reg(npu, (uint32_t) (IADDR_MLX|iaddr_core_idx), 0x40000000);
    }
    return error_observed;
}

int test_int_reg_dma(npu_t* npu)
{
    uint32_t r_cmd, r_data;
    int error_observed = 0;
    volatile uint32_t* dma_addr;

    // Reset
    npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RESET);

    //r_cmd = {OP_WAIT, 28'h0};
    r_cmd = NPU_OPCODE_NOP << 28;
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, r_cmd);

    // 1-1. Fill ones in internal regs
    // DMA
    r_cmd = 0;
    r_cmd |= NPU_OPCODE_WR_REG << 28;
    r_cmd |= 0x0 << 24;
    r_cmd |= 0x05 << 16;
    //r_cmd |= (IADDR_DMA | 0x04);
    r_cmd |= IADDR_DMA;
    
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, r_cmd);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFE);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);

    // Run NPU
    npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RUN);
    
    npu_test_sleep(200);

    // 1-2. Check 
    // Stop NPU
    npu_write_reg(npu, ADDR_NPU_CONTROL, 0x0);

    // Read internal register
    // DMA
    npu_test_printf("DMA_INT_REG(Ones)\r\n");
    //error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x00), 0x00);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x00), 0xe0);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x04), 0xFFFFFFFF);
    if (npu_nd_cs)
        error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x08), 0x30F317D7);
    else
        error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x08), 0x30F317C7);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x0C), 0x7FFF7FFF);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x10), 0x001FFFFF);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x14), 0xFFF01FFF);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x20), 0x0);  // ID

    // 2-1. Fill zeros in internal regs
    // DMA
    r_cmd = 0;
    r_cmd |= NPU_OPCODE_WR_REG << 28;
    r_cmd |= 0x0 << 24;
    //r_cmd |= 0x04 << 16;
    r_cmd |= 0x05 << 16;
    //r_cmd |= (IADDR_DMA | 0x04);
    r_cmd |= IADDR_DMA;
    
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, r_cmd);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);

    // Run NPU
    npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RUN);
    
    npu_test_sleep(200);

    // 2-2. Check 
    // Stop NPU
    npu_write_reg(npu, ADDR_NPU_CONTROL, 0x0);

    // Read internal register
    // DMA
    npu_test_printf("DMA_INT_REG(Zeros)\r\n");
    //error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x00), 0x00);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x00), 0x0);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x04), 0x0);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x08), 0x0);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x0C), 0x0);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x10), 0x0);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x14), 0x0);
    error_observed += check_int_reg(npu, (uint32_t) (IADDR_DMA|0x20), 0x0);

    return error_observed;
}

int test_int_reg_mlx(npu_t* npu)
{
    uint64_t iaddr_core_idx;
    uint32_t cmd;
    uint32_t iaddr;
    int error_observed = 0;

    for (int i = 0; i < npu_num_core; i = i + 1) {
        // Reset
        npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RESET);

        // Core index
        iaddr_core_idx = i<<11;

        // Step1. One-fill test
        //MLX
        iaddr = (uint32_t) (IADDR_MLX|iaddr_core_idx);
        cmd = 0;
        cmd |= NPU_OPCODE_WR_REG << 28;
        cmd |= 0 << 24;
        cmd |= 0xe << 16;
        cmd |= iaddr|0x4;

        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFF0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);

        // Run NPU
        npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RUN);

        npu_test_sleep(1000*100); 

        // Stop NPU
        npu_write_reg(npu, ADDR_NPU_CONTROL, 0x0);

        npu_test_printf("CORE%d MLX INT_REG(Ones)\r\n", i);
        // MLX
        iaddr = (uint32_t) (IADDR_MLX|iaddr_core_idx);
        error_observed += check_int_reg(npu, iaddr|0x00, 0x40000000);
        error_observed += check_int_reg(npu, iaddr|0x04, 0xFFFFFFF0);
        error_observed += check_int_reg(npu, iaddr|0x08, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x0C, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x10, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x14, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x18, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x1C, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x20, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x24, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x28, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x2C, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x30, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x34, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x38, 0xFFFFFFFF);
        
        // Step2. Zero-fill test
        //MLX
        iaddr = (uint32_t) (IADDR_MLX|iaddr_core_idx);
        cmd = 0;
        cmd |= NPU_OPCODE_WR_REG << 28;
        cmd |= 0 << 24;
        cmd |= 0xe << 16;
        cmd |= iaddr|0x4;

        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        
        // Run NPU
        npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RUN);

        npu_test_sleep(200); 

        // Stop NPU
        npu_write_reg(npu, ADDR_NPU_CONTROL, 0x0);

        npu_test_printf("CORE%d MLX INT_REG(Zeros)\r\n", i);
        // MLX
        iaddr = (uint32_t) (IADDR_MLX|iaddr_core_idx);
        error_observed += check_int_reg(npu, iaddr|0x00, 0x40000000);
        error_observed += check_int_reg(npu, iaddr|0x04, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x08, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x0C, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x10, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x14, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x18, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x1C, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x20, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x24, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x28, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x2C, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x30, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x34, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x38, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x3C, 0x0);
    }

    return error_observed;
}

int test_int_reg_other(npu_t* npu)
{
    uint64_t iaddr_core_idx;
    uint32_t cmd;
    uint32_t iaddr;
    int error_observed = 0;

    for (int i = 0; i < npu_num_core; i = i + 1) {
        // Reset
        npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RESET);

        // Core index
        iaddr_core_idx = i<<11;

        // Step1. One-fill test
        // MM
        iaddr = (uint32_t) (IADDR_MM|iaddr_core_idx);
        cmd = 0;
        cmd |= NPU_OPCODE_WR_REG << 28;
        cmd |= 0 << 24;
        
        if (npu_nd_cs)
            cmd |= 0x6 << 16;
        else
            cmd |= 0x5 << 16;

        cmd |= iaddr|0x4;
        
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        if (npu_nd_cs)
            npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);

        cmd = 0;
        cmd |= NPU_OPCODE_RUN << 28;
        cmd |= 0 << 24;
        cmd |= 0x2 << 16;  // reset (5bit)
        cmd |= iaddr;
        
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd);

        // DW
        iaddr = (uint32_t) (IADDR_DW|iaddr_core_idx);
        cmd = 0;
        cmd |= NPU_OPCODE_WR_REG << 28;
        cmd |= 0 << 24;

        if (npu_nd_cs)
            cmd |= 0x4 << 16;
        else
            cmd |= 0x3 << 16;
        cmd |= iaddr|0x4;

        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        if (npu_nd_cs)
            npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        
        cmd = 0;
        cmd |= NPU_OPCODE_RUN << 28;
        cmd |= 0 << 24;
        cmd |= 0x2 << 16;
        cmd |= iaddr;

        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd); // DW internal reset

        // MISC
        iaddr = (uint32_t) (IADDR_MISC|iaddr_core_idx);
        cmd = 0;
        cmd |= NPU_OPCODE_WR_REG << 28;
        cmd |= 0 << 24;
        cmd |= 0x5 << 16;
        cmd |= iaddr|0x4;

        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);

        cmd = 0;
        cmd |= NPU_OPCODE_RUN << 28;
        cmd |= 0 << 24;
        cmd |= 0x2 << 16;
        cmd |= iaddr;
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd); // MISC internal reset

        // Run NPU
        npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RUN);

        npu_test_sleep(200); 

        // Stop NPU
        npu_write_reg(npu, ADDR_NPU_CONTROL, 0x0);
        
        // 1-2. Check 
        // Read internal register

        npu_test_printf("CORE%d MM INT_REG(Ones)\r\n", i);
        // MM
        iaddr = (uint32_t)(IADDR_MM|iaddr_core_idx);
        error_observed += check_int_reg(npu, iaddr|0x00, 0x10000000);
        error_observed += check_int_reg(npu, iaddr|0x04, 0x7FFF1077);
        error_observed += check_int_reg(npu, iaddr|0x08, 0x00007FFF);
        error_observed += check_int_reg(npu, iaddr|0x0C, 0x7FFF7FFF);
        error_observed += check_int_reg(npu, iaddr|0x10, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x14, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x18, 0x7FFF7FFF);

        if (npu_nd_cs)
            error_observed += check_int_reg(npu, iaddr|0x1C, 0x000000FF);

        error_observed += check_int_reg(npu, iaddr|0x20, 0x1);

        npu_test_printf("CORE%d DW INT_REG(Ones)\r\n", i);
        // DW
        iaddr = (uint32_t)(IADDR_DW|iaddr_core_idx);
        error_observed += check_int_reg(npu, iaddr|0x00, 0x20000000);
        if (npu_nd_cs)
            error_observed += check_int_reg(npu, iaddr|0x04, 0x7FFF7FFF);  // new(DW5x5)
        else
            error_observed += check_int_reg(npu, iaddr|0x04, 0x70717FFF);
        error_observed += check_int_reg(npu, iaddr|0x08, 0x7FFF7FFF);
        error_observed += check_int_reg(npu, iaddr|0x0C, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x10, 0x7FFF7FFF);
        if (npu_nd_cs)
            error_observed += check_int_reg(npu, iaddr|0x14, 0x000000FF);
        error_observed += check_int_reg(npu, iaddr|0x20, 0x2);  // ID

        npu_test_printf("CORE%d MISC INT_REG(Ones)\r\n", i);
        // MISC
        iaddr = (uint32_t) (IADDR_MISC|iaddr_core_idx);
        error_observed += check_int_reg(npu, iaddr|0x00, 0x30000000);
        error_observed += check_int_reg(npu, iaddr|0x04, 0x351303FF);
        error_observed += check_int_reg(npu, iaddr|0x08, 0x7FFF7FFF);
        error_observed += check_int_reg(npu, iaddr|0x0C, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x10, 0xFFFFFFFF);
        error_observed += check_int_reg(npu, iaddr|0x14, 0x00FF070F);
        error_observed += check_int_reg(npu, iaddr|0x18, 0x7FFF7FFF);
        error_observed += check_int_reg(npu, iaddr|0x20, 0x3);  // ID


        // Step2. One-zero test
        // MM
        iaddr = (uint32_t) (IADDR_MM|iaddr_core_idx);
        cmd = 0;
        cmd |= NPU_OPCODE_WR_REG << 28;
        cmd |= 0 << 24;
        cmd |= 0x6 << 16;
        cmd |= iaddr|0x4;
        
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);

        cmd = 0;
        cmd |= NPU_OPCODE_RUN << 28;
        cmd |= 0 << 24;
        cmd |= 0x2 << 16;
        cmd |= iaddr;
        
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd); // MM internal reset

        // DW
        iaddr = (uint32_t) (IADDR_DW|iaddr_core_idx);
        cmd = 0;
        cmd |= NPU_OPCODE_WR_REG << 28;
        cmd |= 0 << 24;
        cmd |= 0x4 << 16;
        cmd |= iaddr|0x4;

        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        
        cmd = 0;
        cmd |= NPU_OPCODE_RUN << 28;
        cmd |= 0 << 24;
        cmd |= 0x2 << 16;
        cmd |= iaddr;

        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd); // DW internal reset

        // MISC
        iaddr = (uint32_t) (IADDR_MISC|iaddr_core_idx);
        cmd = 0;
        cmd |= NPU_OPCODE_WR_REG << 28;
        cmd |= 0 << 24;
        cmd |= 0x5 << 16;
        cmd |= iaddr|0x4;

        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0x0);

        cmd = 0;
        cmd |= NPU_OPCODE_RUN << 28;
        cmd |= 0 << 24;
        cmd |= 0x2 << 16;
        cmd |= iaddr;
        npu_write_reg(npu, ADDR_NPU_APB_COMMAND, cmd);

        // Run NPU
        npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RUN);

        npu_test_sleep(200); 

        // Stop NPU
        npu_write_reg(npu, ADDR_NPU_CONTROL, 0x0);
        
        // 2-2. Check 
        // Read internal register

        npu_test_printf("CORE%d MM INT_REG(Zeros)\r\n", i);
        // MM
        iaddr = (uint32_t)(IADDR_MM|iaddr_core_idx);
        error_observed += check_int_reg(npu, iaddr|0x00, 0x10000000);
        error_observed += check_int_reg(npu, iaddr|0x04, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x08, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x0C, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x10, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x14, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x18, 0x0);
        if (npu_nd_cs)
            error_observed += check_int_reg(npu, iaddr|0x1C, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x20, 0x1);  // ID

        npu_test_printf("CORE%d DW INT_REG(Zeros)\r\n", i);
        // DW
        iaddr = (uint32_t)(IADDR_DW|iaddr_core_idx);
        error_observed += check_int_reg(npu, iaddr|0x00, 0x20000000);
        error_observed += check_int_reg(npu, iaddr|0x04, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x08, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x0C, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x10, 0x0);
        if (npu_nd_cs)
            error_observed += check_int_reg(npu, iaddr|0x14, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x20, 0x2);  // ID

        npu_test_printf("CORE%d MISC INT_REG(Zeros)\r\n", i);
        // MISC
        iaddr = (uint32_t) (IADDR_MISC|iaddr_core_idx);
        error_observed += check_int_reg(npu, iaddr|0x00, 0x30000000);
        error_observed += check_int_reg(npu, iaddr|0x04, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x08, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x0C, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x10, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x14, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x18, 0x0);
        error_observed += check_int_reg(npu, iaddr|0x20, 0x3);  // ID

    }

    return error_observed;
}

int test_cmd_q_empty_irq(npu_t* npu)
{
    uint32_t r_cmd;
    int error_observed = 0;

    // Reset
    npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RESET);

    // Add invalid NPU CMD
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);
    npu_write_reg(npu, ADDR_NPU_APB_COMMAND, 0xFFFFFFFF);

    // Enable CMD-Q empty IRQ 
    npu_write_reg(npu, ADDR_NPU_IRQ_ENABLE, 0x1);

    // Run NPU
    npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RUN);
    
    npu_test_sleep(200);

    // Stop NPU
    npu_write_reg(npu, ADDR_NPU_CONTROL, 0x0);

    // Check IRQ (should be 0x1)
    error_observed += apb_read_check(npu, ADDR_NPU_IRQ_REASON, 0x1);

    // Disable IRQ
    npu_write_reg(npu, ADDR_NPU_IRQ_ENABLE, 0x0);

    // Clear IRQ
    npu_write_reg(npu, ADDR_NPU_IRQ_CLEAR, 0x1);

    // Reset
    npu_write_reg(npu, ADDR_NPU_CONTROL, NPU_CTRL_RESET);

    return error_observed;
}

int test_npu_reg(npu_t* npu)
{
    int err_cnt = 0;
    int err_tot = 0;

    err_cnt = test_main_reg(npu);
    err_tot += err_cnt;
    if (err_cnt)
        npu_test_printf("%d, err detected in test_main_reg\n", err_cnt);

    err_cnt = test_hci(npu);
    err_tot += err_cnt;
    if (err_cnt)
        npu_test_printf("%d, err detected in test_hci\n", err_cnt);

    err_cnt = test_int_reg_id(npu);
    err_tot += err_cnt;
    if (err_cnt)
        npu_test_printf("%d, err detected in test_int_reg_id\n", err_cnt);

    err_cnt = test_int_reg_dma(npu);
    err_tot += err_cnt;
    if (err_cnt)
        npu_test_printf("%d, err detected in test_int_reg_dma\n", err_cnt);

    err_cnt = test_int_reg_mlx(npu);
    err_tot += err_cnt;
    if (err_cnt)
        npu_test_printf("%d, err detected in test_int_reg_mlx\n", err_cnt);

    err_cnt = test_int_reg_other(npu);
    err_tot += err_cnt;
    if (err_cnt)
        npu_test_printf("%d, err detected in test_int_reg_other\n", err_cnt);

    err_cnt = test_cmd_q_empty_irq(npu);
    err_tot += err_cnt;
    if (err_cnt)
        npu_test_printf("%d, err detected in test_cmd_q_empty_irq\n", err_cnt);

    return err_tot;
}

int npu_reg_test_main(npu_t* npu)
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

    if (((val >> 16) & 0xFF)>= 0x21)
        npu_nd_cs = 1;
    else
        npu_nd_cs = 0;

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
    npu_test_printf("Register Test Start\n");
    npu_test_printf("========================\n");

    err_cnt = test_npu_reg(npu);

    if (err_cnt) {
        npu_test_printf("fail\n");
        npu_test_printf("Total number of err: %d\r\n", err_cnt);
    }
    else
        npu_test_printf("success\n");

    npu_test_printf("========================\n");
    npu_test_printf("TEST_INFO: DONE\r\n");
    npu_test_printf("========================\n");

    return err_cnt;
}
