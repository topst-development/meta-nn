/**
 *  Openedges Enlight NPU Testapplication
 *
 *  Npu APIs.
 */
#include <stdio.h>
#include "npu_api.h"

static int get_file_size(char *path);

//-------------------------------------------------------------------
// NPU Network
//-------------------------------------------------------------------
npu_net_t* network_load_from_file(
    npu_t*  npu,
    char*   sofile,
    char*   cmdfile,
    char*   paramfile
)
{
    FILE* cmd_fp = NULL;
    char* cmd = NULL;
    int cmd_size = get_file_size(cmdfile);

    FILE* param_fp = NULL;
    char* param = NULL;
    int param_size = get_file_size(paramfile);

    npu_net_t* net = NULL;

    void* dl = NULL;
    // cppcheck-suppress misra-c2012-17.7
    int (*init_net)(struct enlight_net*);
    struct enlight_net* methods = NULL;

    unsigned int c_size;
    unsigned int p_size;

    npu_net_t* ret;
    enlight_api_ret_t status;

    npu_api_fin();

    if ((cmd_size < 0) || (param_size < 0)) {
        status = -ENL_API_NOK_FAIL;
    }
    else {
        status = ENL_API_OK;
    }

    c_size = cmd_size;
    p_size = param_size;

    if (status == ENL_API_OK) {
        // load command
        cmd_fp = fopen(cmdfile, "rb");
        if (!cmd_fp) {
            status = -ENL_API_NOK_FAIL;
        }
    }

    if (status == ENL_API_OK) {
        cmd = npu_api_malloc(sizeof(char)*c_size);
        if (!cmd) {
            status = -ENL_API_NOK_FAIL;
        }
    }

    if (status == ENL_API_OK) {
        if (fread(cmd, sizeof(char), c_size, cmd_fp) < c_size) {
            status = -ENL_API_NOK_FAIL;
        }
    }

    if (status == ENL_API_OK) {
        // load parameter
        param_fp = fopen(paramfile, "rb");
        if (!param_fp) {
            status = -ENL_API_NOK_FAIL;
        }
    }

    if (status == ENL_API_OK) {
        param = npu_api_malloc(sizeof(char)*p_size);
        if (!param) {
            status = -ENL_API_NOK_FAIL;
        }
    }

    if (status == ENL_API_OK) {
        if (fread(param, sizeof(char), p_size, param_fp) < p_size) {
            status = -ENL_API_NOK_FAIL;
        }
    }

    if (status == ENL_API_OK) {
        // open network
        net = network_load(npu, cmd, c_size, param, param_size);
        if (!net) {
            status = -ENL_API_NOK_FAIL;
        }
    }

    if (status == ENL_API_OK) {
        // open linked methods
        dl = dlopen(sofile, RTLD_NOW);
        if (!dl) {
            status = -ENL_API_NOK_FAIL;
        }
    }

    if (status == ENL_API_OK) {
        methods = enlight_net_malloc();
        if (!methods) {
            status = -ENL_API_NOK_FAIL;
        }
    }

    if (status == ENL_API_OK) {
        init_net = dlsym(dl, "init_network");
        init_net(methods);

        net->methods = methods;
        net->dl = dl;

        npu_api_free(cmd);
        npu_api_free(param);

        if (cmd_fp != NULL) {
            (void)fclose(cmd_fp);
        }

        if (param_fp != NULL) {
            (void)fclose(param_fp);
        }

        ret = net;
    }
    else {
        if (net != NULL) {
           (void)network_close(net);
        }

        npu_api_free(cmd);
        npu_api_free(param);
        enlight_net_free(methods);

        if (dl != NULL) {
            dlclose(dl);
        }

        ret = NULL;
    }

    npu_api_fout();

    return ret;
}

//-------------------------------------------------------------------
// Static functions for local usage
//-------------------------------------------------------------------
static int get_file_size(char *path)
{
    FILE *fp;
    int file_size;
    int ret;
    int err;

    if (!access(path, F_OK)) {

        errno = 0;
        fp = fopen(path, "rb");
        err = fseek(fp, 0, SEEK_END);

        if (!err) {
            errno = 0;
            file_size = ftell(fp);
            if (errno != 0) {
                err = -1;
            }
        }

        if (!err) {
            err = fseek(fp, 0, SEEK_SET);
        }

        if (!err) {
            err = fclose(fp);
        }

        if (!err) {
            ret = file_size;
        }
        else {
            ret = -1;
        }
    }
    else {
        ret = -1;
    }

    if (ret < 0) {
        (void)fprintf(stderr, "get_file_size failed %s(%d)\n", \
                __func__, __LINE__);
    }

    return ret;
}
