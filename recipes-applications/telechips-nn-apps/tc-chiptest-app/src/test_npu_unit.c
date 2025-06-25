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

char test_list_cs[][256] = {
    {"conv1x1_p0x0_s1x1_bypass_c2016_k32_w8_h8"},
    {"conv1x1_p0x0_s1x1_bypass_c32_k32_w8_h8"},
    {"conv1x1_p0x0_s1x1_relu_c24_k48_w8_h8_qbit4"},
    {"conv1x1_p0x0_s2x2_bypass_c32_k32_w32_h32"},
    {"conv1x1_p0x0_s2x2_leaky_c32_k32_w32_h32_qbit4"},
    {"conv1x3_p0x1_s1x1_bypass_c32_k32_w32_h32_d1x2"},
    {"conv2x2_p0x0_s1x1_relu_c32_k32_w8_h8"},
    {"conv3x1_p1x0_s1x1_bypass_c32_k32_w32_h32_d2x1"},
    {"conv3x1_p1x0_s2x1_bypass_c31_k31_w8_h8"},
    {"conv3x3_p0x0_s2x2_bypass_c33_k35_w16_h16"},
    {"conv3x3_p0x1_s1x1_relu_c32_k32_w8_h8"},
    {"conv3x3_p1x1_s1x1_bypass_c32_k32_w1024_h8"},
    {"conv3x3_p1x1_s1x1_bypass_c32_k32_w8_h1024"},
    {"conv3x3_p1x1_s1x1_leaky_c96_k32_w8_h8"},
    {"conv3x3_p1x1_s1x1_relu_c32_k32_w16_h16_d3x3"},
    {"conv3x3_p1x1_s1x1_relu_c32_k32_w16_h16_d4x4"},
    {"conv3x3_p1x1_s1x1_relu_c32_k32_w8_h8_d2x2"},
    {"conv3x3_p1x1_s1x1_relu_c32_k32_w8_h8_d2x2_qbit4"},
    {"conv3x5_p2x0_s1x1_bypass_c32_k32_w32_h32_d2x1"},
    {"conv4x4_p2x2_s1x1_relu_c32_k32_w8_h8"},
    {"conv5x1_p2x0_s2x1_bypass_c31_k31_w8_h8"},
    {"conv5x3_p2x0_s1x1_bypass_c32_k32_w32_h32_d1x2"},
    {"conv5x5_p2x0_s1x1_bypass_c32_k32_w32_h32_d1x2"},
    {"conv5x5_p2x0_s1x1_bypass_c32_k32_w32_h32_d2x1"},
    {"conv5x5_p2x2_s1x1_relu_c32_k32_w8_h8"},
    {"conv5x5_p2x2_s2x2_relu_c32_k48_w8_h8"},
    {"conv6x6_p0x0_s2x2_relu_c64_k16_w8_h8"},
    {"conv7x7_p1x1_s4x4_relu_c48_k16_w8_h8"},
    {"dma_act-basic_w1_h1_c32"},
    {"dma_act-basic_w32_h128_c64"},
    {"dma_act-basic_w32_h128_c64_goffset16384"},
    {"dma_act-basic_w32_h128_c64_goffset24576"},
    {"dma_act-basic_w32_h128_c64_goffset8192"},
    {"dma_act-basic_w4_h4_c32_offset3840"},
    {"dma_act-input_w1024_h4_c3"},
    {"dma_act-input_w20_h4_c3_im2col1_sinv"},
    {"dma_act-input_w20_h4_c3_sinv"},
    {"dma_act-input_w30_h4_c3_im2col2_sinv"},
    {"dma_act-input_w8_h2_c1"},
    {"dma_act-input_w8_h2_c1_im2col1"},
    {"dma_act-input_w8_h2_c1_im2col2"},
    {"dma_act-input_w8_h2_c2"},
    {"dma_act-input_w8_h2_c2_im2col1"},
    {"dma_act-input_w8_h2_c2_im2col2"},
    {"dma_act-input_w8_h2_c3"},
    {"dma_act-input_w8_h2_c3_im2col1"},
    {"dma_act-input_w8_h2_c3_im2col2"},
    {"dma_act-input_w8_h2_c4"},
    {"dma_act-input_w8_h2_c4_im2col1"},
    {"dma_act-input_w8_h2_c4_im2col2"},
    {"dma_act-output_w2_h2_c4096"},
    {"dma_act-output_w5_h4_c16"},
    {"dma_act-output_w5_h4_c4"},
    {"dma_act-output_w5_h4_c8"},
    {"dma_act-partial_w1024_h8_c32"},
    {"dma_act-partial_w4_h1024_c64"},
    {"dma_gbuf-basic_w1024_h8_c32"},
    {"dma_gbuf-basic_w8_h4_c32_goffset120"},
    {"dma_gbuf-partial_w1024_h8_c32"},
    {"dma_gbuf-partial_w4_h2048_c32"},
    {"dw3x3_p0x0_s1x1_bypass_c4096_k4096_w8_h8"},
    {"dw3x3_p0x0_s1x1_relu_c32_k32_w8_h8"},
    {"dw3x3_p0x0_s2x2_relu_c64_k64_w8_h8"},
    {"dw3x3_p1x1_s1x1_bypass_c96_k96_w8_h8"},
    {"dw3x3_p1x1_s2x2_leaky_c64_k64_w8_h8"},
    {"pool_ce0_mo-avg_c32_w5_h4"},
    {"pool_ce0_mo-avg_c32_w8_h3"},
    {"pool_ce0_mo-max_c32_w1024_h8"},
    {"pool_ce0_mo-max_c32_w16_h7"},
    {"pool_ce0_mo-max_c32_w8_h1024"},
    {"pool_ce1_mo-avg_c32_w5_h4"},
    {"pool_ce1_mo-avg_c32_w8_h3"},
    {"pool_ce1_mo-max_c32_w8_h8"},
    {"reducebw_c120_w8_h8_t6_cs7"},
    {"reducebw_c192_w8_h8_t8_cs31"},
    {"reducebw_c32_w8_h8_t15_cs255"},
    {"scalar_em0_bypass_t0_cs1_cml02_ca02_c32_w1024_h8"},
    {"scalar_em0_bypass_t4_cs1_cml-4_ca03_c32_w4_h5"},
    {"scalar_em0_mishres_t0_cs1_cml1_ca0_c32_w8_h8"},
    {"scalar_em0_relu_t0_cs1_cml1_ca0_c32_w8_h8"},
    {"scalar_em0_sigmoid_t0_cs1_cml1_ca0_c32_w8_h8"},
    {"scalar_em0_tanh_t0_cs1_cml1_ca0_c32_w8_h8"},
    {"scalar_em1_bypass_t0_cs1_cml01_cmr01_c32_w1024_h8"},
    {"scalar_em2_bypass_t0_cs1_c32_w4_h4"},
    {"scalar_em3_bypass_t13_cs167_c32_w7_h7"},
    {"upscale_ty-nn_c32_w1024_h2"},
    {"upscale_ty-nn_c32_w2_h1024"},
    {"upscale_ty-nn_c32_w7_h8"},
    {"upscale_ty-nn_c96_w4_h4"},
    {"upscale_ty-zf_c64_w16_h16"},
};

int test_npu_unit(npu_t* npu)
{
    char net_path[512];
    char ref_path[512];
    char *cmd_file_name = NULL;
    char *weight_file_name = NULL;
    char *input_file_name = NULL;
    char *so_file_name = NULL;
    int i;
    unsigned int val;
    unsigned int npu_nd_cs = 0;

    npu_buf_t* in = NULL;
    npu_buf_t* out = NULL;
    npu_net_t* net = NULL;

    int test_result = NPU_TEST_FAIL;


    so_file_name = malloc(512 * sizeof(char));
    cmd_file_name = malloc(512 * sizeof(char));
    weight_file_name = malloc(512 * sizeof(char));
    input_file_name = malloc(512 * sizeof(char));

    if (!so_file_name || !cmd_file_name || !weight_file_name || !input_file_name) {
        npu_test_printf("malloc fail\n");
        goto test_fail;
    }

    int test_num;
    char test_data_dir_cs[256] = "";
    char test_data_dir_es[256] = "";
    char *test_data_dir;

    npu_read_reg(npu, ADDR_NPU_ID_CODE, &val);

    npu_num_core   = (val >> 24) & 0xF;
    npu_major_ver  = (val >> 20) & 0xF;
    npu_minor_ver  = (val >> 18) & 0x3;
    npu_product_id = (val >>  0) & 0xFFFF;
    npu_ecc        = (val >> 16) & 0x1;
    npu_num_core   += 1;

    npu_test_printf("========================\n");
    npu_test_printf("NPU product id: 0x%04X\n", npu_product_id);
    npu_test_printf("NPU major ver : 0x%X\n", npu_major_ver);
    npu_test_printf("NPU minor ver : 0x%X\n", npu_minor_ver);
    npu_test_printf("NPU ecc       : 0x%X\n", npu_ecc);
    npu_test_printf("NPU core num  : 0x%X\n", npu_num_core);
    npu_test_printf("========================\n");

    npu_test_printf("========================\n");
    npu_test_printf("NPU  Unit Fucntion Test\n");
    npu_test_printf("========================\n");

    if (((val >> 16) & 0xFF) < 0x21) {
        npu_test_printf("Not Ndolphin CS\n");
        return 1;
    }

    //CS
    test_data_dir = test_data_dir_cs;
    test_num = sizeof(test_list_cs)/ sizeof(test_list_cs[0]);

    for (i = 0; i < test_num; i++) {
        char* test_name;
        int input_size;
        int output_size;
        npu_net_t* net = NULL;
        npu_buf_t* in = NULL;
        npu_buf_t* out = NULL;

        test_name = test_list_cs[i];
        npu_test_printf("%s start \n", test_name);

        sprintf(net_path, "%s/output_code/%s", test_data_dir, test_list_cs[i]);
        sprintf(ref_path, "%s/output_dump/%s/ref_data", test_data_dir, test_list_cs[i]);


        sprintf(so_file_name,     "%s/net.so", net_path);
        sprintf(cmd_file_name,    "%s/npu_cmd.bin", net_path);
        sprintf(weight_file_name, "%s/quantized_network.bin", net_path);
        sprintf(input_file_name,  "%s/input.ia.bin", ref_path);

        if (get_file_size(weight_file_name) <= 0) {
            //weight file not exist in dma test
            sprintf(weight_file_name, "./quantized_network.bin");
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

        in = buffer_alloc(npu, input_size);
        load_input(in, input_file_name, input_size);

        // prepare output buffer
        output_size = network_get_output_size(net);
        out = buffer_alloc(npu, output_size);

#ifdef ENLIGHT_DEBUG
        npu_test_printf("%s - start inferencing\n", test_name);
#endif

        // run network
        if (network_run(net, in, out, TIMEOUT_IN_MS) < 0)
        {
            npu_test_printf("%s - inference failed\n", test_name);
            goto test_fail;
        }

#ifdef ENLIGHT_DEBUG
        npu_test_printf("%s - inference done\n", test_name);
#endif

        if (out_verify(out, net->methods, ref_path)) {
            npu_test_printf("verification fail: %s \n", test_name);
            goto test_fail;
        }
        else
            npu_test_printf("test pass \n");

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
        return 1;
    }

    npu_test_printf("success\n");
    return 0;
}

