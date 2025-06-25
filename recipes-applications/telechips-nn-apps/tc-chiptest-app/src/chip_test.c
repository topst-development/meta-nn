/**
 *  Openedges Enlight NPU Testapplication
 *
 *  NPU Verification application registert test function.
*/
#include <stdio.h>
#include <unistd.h>

#include "chip_test.h"
#include "npu_reg.h"

void npu_test_sleep(int us)
{
    usleep(us);
}

int load_input(npu_buf_t* in, char* input_file_name, int size)
{
    FILE* fp = NULL;
    int8_t* addr = (int8_t*)buffer_get_addr(in);

    fp = fopen(input_file_name, "rb");
    if (fp == NULL) {
        fprintf(stderr, "file open failed %s in %s\n", input_file_name, __func__);
        exit(1);
    }
    fread(addr, sizeof(int8_t), size, fp);
    fclose(fp);

    return 0;
}

int get_file_size(char *path)
{
    FILE *fp = NULL;
    int file_size;

    if (access(path, F_OK))
         return -1;

    fp = fopen(path, "rb");

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    fclose(fp);

    return file_size;
}

int out_verify(npu_buf_t* out, struct enlight_net* net_method, char *ref_path) 
{
    FILE *fp_ref = NULL;
    char *ref_buf = NULL;
    int ref_size, tensor_num;
    int off, size;
    int i;
    char file_path[512]; 
    int test_ret = -1;

    char* outbuf = buffer_get_addr(out);

    tensor_num = net_method->get_output_tensor_num();


    for (i = 0; i < tensor_num; i++) {
        char* name;
        int ret;
        name = net_method->get_tensor_name(i);
        size = net_method->get_tensor_size(i);
        off = net_method->get_tensor_offset(i);

        sprintf(file_path,"%s/sample/%s.oa.bin", ref_path, name);
        ref_size = get_file_size(file_path);

#ifdef ENLIGHT_DEBUG
        printf("out: size:off:name = %8d %8d %s\n", size, off, name);
        printf("ref: size:off:name = %8d %8d %s\n", ref_size, 0, file_path);
#endif
        fp_ref = fopen(file_path, "ro");
        if (!fp_ref) {
            printf(" ref file open fail %s\n", file_path);
            goto test_fail;
        }
      
        ref_buf = (char *)malloc(ref_size * sizeof(char));
        if (!ref_buf) {
            printf(" ref buf alloc fail\n");
            goto test_fail;
        }

        ret = fread(ref_buf, 1, ref_size, fp_ref);
        if (ret < 0) {
            printf(" ref read fail\n");
            goto test_fail;
        }

        ret = memcmp(ref_buf, outbuf + off, size);

        if (ref_buf) {
            free(ref_buf);
            ref_buf = NULL;
        }
        if (fp_ref) {
            fclose(fp_ref);
            fp_ref = NULL;
        }

        if (ret) {
            printf("out != ref, verificaiton fail!\n");
            goto test_fail;
        }
        //else {
        //    printf("[%d]: %s verified\n", i, file_path);
        //}
    }

    test_ret = 0;

test_fail:
    if (ref_buf)
        free(ref_buf);
    if (fp_ref)
        fclose(fp_ref);

    return test_ret;
}

void npu_print_main_regs(npu_t* npu)
{
    unsigned int data, base, i;

    for(i = 0; i < 64; i++) { 
        base = i * 4;
        npu_read_reg(npu, base, &data);
        npu_test_printf("[RD] 0x%08x:0x%08x: NPU main reg \r\n", base, data);
    }
}


void npu_print_hci_regs(npu_t* npu, int core_id)
{
    unsigned int data, base;
    int i;

    base = ADDR_NPU_MLX_C0_HCI_00 + (core_id * 0x40);

    for (i = 0; i < 16; i++) {
        npu_read_reg(npu, base + (i*4), &data);
        npu_test_printf("[RD] 0x%08x:0x%08x: C%d_HCI_%02x\r\n",
                base + (i*4), data, core_id, (i*4));
    }
}
