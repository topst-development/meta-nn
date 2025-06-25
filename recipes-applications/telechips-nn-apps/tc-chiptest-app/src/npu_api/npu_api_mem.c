
/**
 *  Openedges Enlight NPU APIs
 *
 *  NPU API for momory allocation/free.
 */
/**
 * @file    npu_api_mem.c
 * @brief   NPU APIs for memory allocation and free
 *
 *              char*       npu_api_malloc(int size);
 *              void        npu_api_free(void *addr);
 *              npu_buf_t*  npu_buf_malloc(void);
 *              void        npu_buf_free(void* addr);
 *              npu_net_t*  npu_net_malloc(void);
 *              void        npu_net_free(void *addr);
 *              struct enlight_net*
 *                          enlight_net_malloc(void);
 *              void        enlight_net_free(void *addr);
 *
 *          NPU APIs programmer can implement static memory allocation
 *          by means of replacing malloc/free of stdlib.h
 */

#include <stdlib.h>
#include "npu_api.h"

char* npu_api_malloc(int size)
{
    char* addr;

    npu_api_fin();

    addr = (char*)malloc(size);

    npu_api_fout();

    return addr;
}

void npu_api_free(void *addr)
{
    npu_api_fin();

    free(addr);

    npu_api_fout();
}

npu_buf_t* npu_buf_malloc(void)
{
    npu_buf_t* addr;

    npu_api_fin();

    addr = (npu_buf_t*)malloc(sizeof(npu_buf_t));

    npu_api_fout();

    return addr;
}

void npu_buf_free(void* addr)
{
    npu_api_fin();

    free(addr);

    npu_api_fout();
}

npu_net_t* npu_net_malloc(void)
{
    npu_net_t* addr;

    npu_api_fin();

    addr = (npu_net_t*)malloc(sizeof(npu_net_t));

    npu_api_fout();

    return addr;
}

void npu_net_free(void *addr)
{
    npu_api_fin();

    free(addr);

    npu_api_fout();
}

struct enlight_net* enlight_net_malloc(void)
{
    struct enlight_net* addr;

    npu_api_fin();

    addr = (struct enlight_net *)malloc(sizeof(struct enlight_net));

    npu_api_fout();

    return addr;
}

void enlight_net_free(void *addr)
{
    npu_api_fin();

    free(addr);

    npu_api_fout();
}
