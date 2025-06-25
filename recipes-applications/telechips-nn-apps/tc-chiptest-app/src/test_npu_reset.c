/**
 *Openedges Enlight NPU Testapplication
 *
 *NPU Verification application npu dma, gbuf, calc
*/
#include <stdio.h>
#include "npu.h"
#include "npu_api.h"
#include "npu_reg.h"
#include "chip_test.h"

static unsigned int npu_num_core = 0;
static unsigned int npu_major_ver = 0;
static unsigned int npu_minor_ver = 0;
static unsigned int npu_product_id = 0;
static unsigned int npu_ecc = 0;

struct reset_network_list {
    char name[255];
    unsigned int dma_ref;
    unsigned int comp_ref;
    unsigned int hangup; //if 1, hangup network
};

static struct reset_network_list reset_net_list[] = {
    {.name = "yolov4_reset",                                      .dma_ref = 5401625, .comp_ref = 8008437,  .hangup = 1},
    {.name = "ssdlite300_quantized",                              .dma_ref = 409881,  .comp_ref = 611324,   .hangup = 0},
    {.name = "yolov4_reset",                                      .dma_ref = 5401625, .comp_ref = 8008437,  .hangup = 1},
    {.name = "yolov3-relu6-voc_quantized",                        .dma_ref = 6213799, .comp_ref = 7483465,  .hangup = 0},
    {.name = "yolov4_reset",                                      .dma_ref = 5401625, .comp_ref = 8008437,  .hangup = 1},
    {.name = "yolov4-waymo100k_3c_640_TDN_BNH_MIN32CH_quantized", .dma_ref = 5401625, .comp_ref = 8008437,  .hangup = 0},
    {.name = "yolov4_reset",                                      .dma_ref = 5401625, .comp_ref = 8008437,  .hangup = 1},
    {.name = "yolov4-waymo100k_3c_800_TDN_BNH_MIN32CH_quantized", .dma_ref = 7598888, .comp_ref = 12890035, .hangup = 0},
};

int test_npu_reset(npu_t* npu)
{
    char net_path[512];
    char ref_path[512];
    char *cmd_file_name = NULL;
    char *weight_file_name = NULL;
    char *input_file_name = NULL;
    char *so_file_name = NULL;
    int i;
    npu_buf_t* in = NULL;
    npu_buf_t* out = NULL;
    npu_net_t* net = NULL;

    char *test_data_dir;
    unsigned int val;

    int test_result = NPU_TEST_FAIL;

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

    npu_test_printf("========================\n");
    npu_test_printf("NPU product id: 0x%04X\n", npu_product_id);
    npu_test_printf("NPU major ver : 0x%X\n", npu_major_ver);
    npu_test_printf("NPU minor ver : 0x%X\n", npu_minor_ver);
    npu_test_printf("NPU ecc       : 0x%X\n", npu_ecc);
    npu_test_printf("NPU core num  : 0x%X\n", npu_num_core);
    npu_test_printf("========================\n");

    npu_test_printf("========================\n");
    npu_test_printf("NPU Reset Test\n");
    npu_test_printf("========================\n");

    so_file_name = malloc(512 * sizeof(char));
    cmd_file_name = malloc(512 * sizeof(char));
    weight_file_name = malloc(512 * sizeof(char));
    input_file_name = malloc(512 * sizeof(char));

    if (!so_file_name || !cmd_file_name || !weight_file_name || !input_file_name) {
        npu_test_printf("malloc fail\n");
        goto test_fail;
    }

    test_data_dir = "";

    // npu_ecc_t ecc_para;
    // npu_chiptest_t ct_para;

    // ecc_para.disable_ecc_ue = 0x0;
    // ecc_para.disable_ecc_ce = 0x0;
    // ecc_para.disable_wdt = 0;

    // ct_para.dev_mlx_bin_idx = 0;
    // ct_para.dev_wdt_ext_cnt = 0;
    // ct_para.dev_wdt_int_cnt = 0;
    // ct_para.dev_ecc_test_ctrl = 0;
    // ct_para.dev_mlx_err_inj_mask_data = 0;
    // ct_para.dev_mlx_err_inj_mask_par = 0;

    if (npu_reset(npu, 0, NULL, NULL)) {
        npu_test_printf("npu_reset() fail\n");
        goto test_fail;
    }

    for (i = 0; i < sizeof(reset_net_list) / sizeof(reset_net_list[0]); i++) {
        char* test_name;
        int input_size;
        int output_size;
        npu_net_t* net = NULL;
        npu_buf_t* in = NULL;
        npu_buf_t* out = NULL;

        test_name = reset_net_list[i].name;
        npu_test_printf("%s start \n", test_name);

        sprintf(net_path, "%s%s", test_data_dir, test_name);
        sprintf(ref_path, "%s%s", test_data_dir, test_name);

        sprintf(so_file_name,     "%s/net.so", net_path);
        sprintf(cmd_file_name,    "%s/npu_cmd.bin", net_path);
        sprintf(weight_file_name, "%s/quantized_network.bin", net_path);
        sprintf(input_file_name,  "%s/input.ia.bin", ref_path);

        if (get_file_size(weight_file_name) <= 0) {
            npu_test_printf("wrong network weight file: %s\n", weight_file_name);
        }

#ifdef ENLIGHT_DEBUG
        npu_test_printf("so    : %s\n", so_file_name);
        npu_test_printf("cmd   : %s\n", cmd_file_name);
        npu_test_printf("wght  : %s\n", weight_file_name);
        npu_test_printf("iact  : %s\n", input_file_name);
#endif
        net = network_load_from_file(npu, so_file_name, cmd_file_name, weight_file_name);
        if (!net) {
            npu_test_printf("%s - network loading failed\n", test_name);
            goto test_fail;
        }
#ifdef ENLIGHT_DEBUG
        npu_test_printf("%s - %s loaded\n", test_name, net->methods->get_network_name());
#endif
        // load input
        input_size = network_get_input_size(net);

#ifdef ENLIGHT_DEBUG
        npu_test_printf("%d: input_size", input_size);
#endif

        in = buffer_alloc(npu, input_size);

        load_input(in, input_file_name, input_size);

        // prepare output buffer
        output_size = network_get_output_size(net);
        out = buffer_alloc(npu, output_size);

#ifdef ENLIGHT_DEBUG
        npu_test_printf("%s - start inferencing\n", test_name);
#endif

        // run network
        if (reset_net_list[i].hangup == 0) {
            if (network_run(net, in, out, TIMEOUT_IN_MS) < 0) {
                npu_test_printf("%s - inference failed\n", test_name);
                goto test_fail;
            }
        }
        else {
            // Reset test
            if (network_run(net, in, out, TIMEOUT_IN_MS) >= 0) {
                npu_test_printf("%s - Test failed\n", test_name);
                goto test_fail;
            }
            else {
                npu_test_printf("%s - inference failed\n", test_name);
                npu_test_printf("%s - Test successed\n", test_name);
            }

            // npu_ecc_t ecc_para;
            // npu_chiptest_t ct_para;

            // ecc_para.disable_ecc_ue = 0x0;
            // ecc_para.disable_ecc_ce = 0x0;
            // ecc_para.disable_wdt = 0;

            // ct_para.dev_mlx_bin_idx = 0;
            // ct_para.dev_wdt_ext_cnt = 0;
            // ct_para.dev_wdt_int_cnt = 0;
            // ct_para.dev_ecc_test_ctrl = 0;
            // ct_para.dev_mlx_err_inj_mask_data = 0;
            // ct_para.dev_mlx_err_inj_mask_par = 0;

            npu_reset(npu, 0, NULL, NULL);

        }

#ifdef ENLIGHT_DEBUG
        npu_test_printf("%s - inference done\n", test_name);
#endif

        if (reset_net_list[i].hangup == 0) {
            if (out_verify(out, net->methods, ref_path)) {
                npu_test_printf("verification fail: %s \n", test_name);
                goto test_fail;
            }
            else
                npu_test_printf("test pass \n");

        }
        else {
            npu_test_printf("verification skip: %s \n", test_name);
        }


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

    }

    test_result = NPU_TEST_SUCCESS;

test_fail:
    if (net)
        network_close(net);
    if (in)
        buffer_close(in);
    if (out)
        buffer_close(out);

    if (so_file_name)
        free(so_file_name);
    if (cmd_file_name)
        free(cmd_file_name);
    if (weight_file_name)
        free(weight_file_name);
    if (input_file_name)
        free(input_file_name);

    if (test_result == NPU_TEST_FAIL) {
        npu_test_printf("fail\n");
        npu_test_printf("========================\n\n");
        return 1;
    }
    else {
        npu_test_printf("success\n");
        npu_test_printf("========================\n\n");
        return 0;
    }
}
