/*
 * openedges chip test
 *
 * Copyright (C) 2020 Openedges
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __CHIP_TEST_H__
#define __CHIP_TEST_H__

#include <stdio.h>
#include <stdint.h>

#include "npu_api.h"

//#define ENLIGHT_DEBUG
#define TIMEOUT_IN_MS 1000

enum
{
    NPU_TEST_SUCCESS = 0x0,
    NPU_TEST_FAIL    = 0x1,
};

// chip_test.c
int get_file_size(char *path);
int load_input(npu_buf_t* in, char* inputfile, int size);
int out_verify(npu_buf_t* out, struct enlight_net *net_method, char *ref_path);
//unsigned int NPU_WRITE_REG(int fd, uint32_t addr, uint32_t data);
//unsigned int NPU_READ_REG(int fd, uint32_t addr);

// test_npu_ecc.c
int npu_ecc_test_main(npu_t* npu, int quick);
// test_npu_reg.c
int npu_reg_test_main(npu_t* npu);
// test_npu_mlx.c
int npu_mlx_test_main(npu_t* npu);
// test_npu_wdt.c
int npu_wdt_test_main(npu_t* npu);

int test_npu_unit(npu_t* npu);
int test_npu_network(npu_t* npu);
int test_npu_reset(npu_t* npu);

void npu_print_main_regs(npu_t* npu);
void npu_print_hci_regs(npu_t* npu, int core_id);

#define npu_test_printf(...) printf(__VA_ARGS__)

#endif
