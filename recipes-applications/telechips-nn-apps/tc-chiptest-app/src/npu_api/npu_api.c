/**
 *  Openedges Enlight NPU APIs
 *
 */
#include <poll.h>
#include <sys/time.h>

#include "npu.h"
#include "npu_api.h"

//#define INFERENCE_ELAPSED_TIME
#ifdef INFERENCE_ELAPSED_TIME
static void inf_time_eval(int end)
{
    static struct timeval t_begin;
    static struct timeval t_end;

    if (!end) {
        gettimeofday(&t_begin, 0);
    }
    else {
        double sec;
        double usec;
        double elap;
        gettimeofday(&t_end, 0);
        sec = t_end.tv_sec - t_begin.tv_sec;
        usec = t_end.tv_usec - t_begin.tv_usec;
        elap = (sec * 1e3) + (usec * 1e-3);
        npu_api_print(stderr, "inference time: %.3f msec\n", elap);
    }
}
#else
#   define inf_time_eval(end)   do {} while(0)
#   define inf_time_eval(end)   do {} while(0)
#endif

//-------------------------------------------------------------------
// NPU APIs
//-------------------------------------------------------------------
#define NUM_NPU_DEV 2

npu_t* npu_open(int minor)
{
    static npu_t npu_devs[NUM_NPU_DEV];

    npu_t* npu;

    npu_api_fin();

    if ((minor == 0) || (minor == 1)) {
        npu = &npu_devs[minor];

        if (minor == 0) {
            npu->fd = open("/dev/npu0", O_RDWR);
        }
        else {
            npu->fd = open("/dev/npu1", O_RDWR);
        }

        if (npu->fd < 0) {
            npu = NULL;
        }
    }
    else {
        npu = NULL;
        npu_api_print("%d:invalid minor value", minor);
    }

    if (!npu) {
        npu_api_print("failed");
    }

    npu_api_fout();

    return npu;
}

int npu_reset(
    npu_t* npu, 
    int soft_reset,
    npu_ecc_t* ecc_param,
    npu_chiptest_t* chiptest_param
)
{
    struct npu_init_req req;

    enlight_api_ret_t ret;

    npu_api_fin();

    if (!chiptest_param) {
        req.soft_reset = 0;
        req.disable_ue_fail = 0;
        req.disable_ce_fail = 0;
        req.disable_wdt = 0;
    }
    else {
        req.soft_reset = soft_reset; 
        req.disable_ue_fail = ecc_param->disable_ue_fail;
        req.disable_ce_fail = ecc_param->disable_ce_fail;
        req.disable_wdt = ecc_param->disable_wdt;
    }

    // for chip test
    if (!chiptest_param) {
        req.dev_mlx_bin_idx = 0;
        req.dev_wdt_ext_cnt = 0;
        req.dev_wdt_int_cnt = 0;
        req.dev_ecc_test_ctrl = 0;
        req.dev_mlx_err_inj_mask_data = 0;
        req.dev_mlx_err_inj_mask_par  = 0;
    }
    else {
        req.dev_mlx_bin_idx = chiptest_param->dev_mlx_bin_idx;
        req.dev_wdt_ext_cnt = chiptest_param->dev_wdt_ext_cnt;
        req.dev_wdt_int_cnt = chiptest_param->dev_wdt_int_cnt;
        req.dev_ecc_test_ctrl = chiptest_param->dev_ecc_test_ctrl;
        req.dev_mlx_err_inj_mask_data = chiptest_param->dev_mlx_err_inj_mask_data;
        req.dev_mlx_err_inj_mask_par = chiptest_param->dev_mlx_err_inj_mask_par;
    }

    if ((!npu) || (npu->fd < 0)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        if (!ioctl(npu->fd, NPU_IOCTL_RESET_NPU, &req)) {
            ret = ENL_API_OK;
        }
        else {
            ret = -ENL_API_NOK_FAIL;
        }
    }

    npu_api_fout();

    return ret;
}

int npu_close(npu_t* npu)
{
    enlight_api_ret_t ret;

    npu_api_fin();

    if ((!npu) || (npu->fd < 0)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        ret = ENL_API_OK;
        close(npu->fd);
    }

    npu_api_fout();

    return ret;
}

int npu_write_reg(
    npu_t*          npu,
    unsigned int    addr,
    unsigned int    data
)
{
    struct reg_access_req req;
    enlight_api_ret_t ret;

    npu_api_fin();

    if ((!npu) || (npu->fd < 0)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        req.addr = addr;
        req.data = data;

        if (!ioctl(npu->fd, NPU_IOCTL_WRITE_REG, &req)) {
            ret = ENL_API_OK;
        }
        else {
            ret = -ENL_API_NOK_FAIL;
        }
    }

    npu_api_fout();

    return ret;
}

int npu_read_reg(
    npu_t*          npu,
    unsigned int    addr,
    unsigned int*   data
)
{
    struct reg_access_req req;
    enlight_api_ret_t ret;

    npu_api_fin();

    if ((!npu) || (npu->fd < 0)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        req.addr = addr;

        if (!ioctl(npu->fd, NPU_IOCTL_READ_REG, &req)) {
            *data = req.data;
            ret = ENL_API_OK;
        }
        else {
            ret = -ENL_API_NOK_FAIL;
        }
    }

    npu_api_fout();

    return ret;
}

//-------------------------------------------------------------------
// NPU Network
//-------------------------------------------------------------------
npu_net_t* network_load(
    npu_t*  npu,
    char*   cmd,
    int     cmd_size,
    char*   param,
    int     param_size
)
{
    npu_net_t* net;

    npu_api_fin();

    if ((!npu) || (npu->fd < 0) ||
        (!cmd) || (cmd_size <= 0) ||
        (!param) || (param_size <= 0)) {
        net = NULL;
    }
    else {
        int fd;
        struct net_load_req req;

        req.cmd_data = cmd;
        req.cmd_size = cmd_size;

        req.wei_data = param;
        req.wei_size = param_size;

        fd = ioctl(npu->fd, NPU_IOCTL_LOAD_NETWORK, &req);

        if (fd >= 0) {
            //net = npu_api_mem_alloc(sizeof(npu_net_t));
            net = npu_net_malloc();
            if (!net) {
                close(fd);
            } else {
                net->fd = fd;
                net->methods = NULL;
                net->dl = NULL;
            }
        }
        else {
            net = NULL;
        }
    }

    npu_api_fout();

    return net;
}

int network_set_color_format(
    npu_net_t*  net,
    int         color
)
{
    enlight_api_ret_t ret;

    npu_api_fin();

    if ((!net) || (net->fd < 0) || (color >= NPU_COLOR_END)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        ret = ioctl(net->fd, NPU_NET_IOCTL_SET_COLOR_FMT, color);
        if (!ret) {
            ret = ENL_API_OK;
        }
        else {
            ret = -ENL_API_NOK_FAIL;
        }
    }

    npu_api_fout();

    return ret;
}

int network_run(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    int         timeout_in_ms
)
{
    enlight_api_ret_t ret;

    npu_api_fin();

    inf_time_eval(0);

    ret = network_issue_run(net, in, out);

    if (ret == ENL_API_OK) {
        ret = network_wait_done(net, timeout_in_ms);
        inf_time_eval(1);
    }

    npu_api_fout();

    return ret;
}

int network_issue_run(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out
)
{
    struct net_run_req req;
    enlight_api_ret_t ret;

    npu_api_fin();

    if ((!net) || (net->fd < 0) ||
        (!in) || (in->fd < 0) ||
        (!out) || (out->fd < 0)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        req.in_fd = in->fd;
        req.out_fd = out->fd;

        if (!ioctl(net->fd, NPU_NET_IOCTL_RUN, &req)){
            ret = ENL_API_OK;
        }
        else {
            ret = -ENL_API_NOK_FAIL;
        }
    }

    npu_api_fout();

    return ret;
}

int network_wait_done(
    npu_net_t*  net,
    int         timeout_in_ms
)
{
    enlight_api_ret_t ret;
    int poll_ret;

    npu_api_fin();

    if ((!net) || (net->fd < 0) || (timeout_in_ms < 0)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        struct pollfd fd;

        fd.fd = net->fd;
        fd.events = POLLIN | POLLERR;
        fd.revents = 0;

        do {
            poll_ret = poll(&fd, 1, timeout_in_ms);
        // cppcheck-suppress misra-c2012-22.10; poll is errno setting func
        } while ((poll_ret < 0) && (errno == EINTR));

        if ((fd.revents & POLLIN) != 0) {
            ret = ENL_API_OK;
        }
        else {
            ret = -ENL_API_NOK_FAIL;
        }
    }

    npu_api_fout();

    return ret;
}

int network_profile(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    npu_perf_t* perf
)
{
    struct net_profile_req req;
    enlight_api_ret_t ret;

    npu_api_fin();

    if ((!net) || (net->fd < 0) ||
        (!in) || (in->fd < 0) ||
        (!out) || (out->fd < 0) ||
        (!perf)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        req.in_fd = in->fd;
        req.out_fd = out->fd;

        if (!ioctl(net->fd, NPU_NET_IOCTL_PROFILE, &req)) {
            perf->elapsed_in_us = req.elapsed_in_us;
            perf->dma = req.dma;
            perf->comp = req.comp;
            perf->all = req.all;

            ret = ENL_API_OK;
        }
        else {
            ret = ENL_API_NOK_FAIL;
        }
    }

    npu_api_fout();

    return ret;
}

int network_get_input_size(npu_net_t* net)
{
    enlight_api_ret_t ret;

    if ((!net) || (!net->methods)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        ret = net->methods->input_size;
    }

    return ret;
}

int network_get_output_size(npu_net_t* net)
{
    int ret;

    if ((!net) || (!net->methods)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        ret = net->methods->output_size;
    }

    return ret;
}

int network_get_input_width(npu_net_t* net)
{
    int ret;

    if ((!net) || (!net->methods)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        ret = net->methods->img_size[2];
    }

    return ret;
}

int network_get_input_height(npu_net_t* net)
{
    int ret;

    if ((!net) || (!net->methods)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        ret = net->methods->img_size[1];
    }

    return ret;
}

int network_get_type(npu_net_t* net)
{
    int ret;

    if ((!net) || (!net->methods)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        ret = net->methods->post_type;
    }

    return ret;
}

int network_run_postprocess(
    npu_net_t* net,
    npu_buf_t* out,
    void* result
)
{
    enlight_api_ret_t ret;

    npu_api_fin();

    if ((!net) || (!net->methods) || (!out) || (!result)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        int batch_size = net->methods->batch_size;
        char* buf = buffer_get_addr(out);

        net->methods->run_post(buf, batch_size, result);

        ret = ENL_API_OK;
    }

    npu_api_fout();

    return ret;
}

int network_close(npu_net_t* net)
{
    enlight_api_ret_t ret;

    npu_api_fin();

    if ((!net) || (net->fd < 0)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        close(net->fd);

        if (net->dl != NULL) {
            dlclose(net->dl);
        }
        ret = ENL_API_OK;

        enlight_net_free(net->methods);
        npu_net_free(net);
    }

    npu_api_fout();

    return ret;
}

//-------------------------------------------------------------------
// NPU Buffer
//-------------------------------------------------------------------
npu_buf_t* buffer_alloc(npu_t* npu, int size)
{
    npu_buf_t* buf;

    npu_api_fin();

    if ((!npu) || (npu->fd < 0) || (size <= 0)) {
        buf = NULL;
    }
    else {
        //buf = npu_api_mem_alloc(sizeof(npu_buf_t));
        buf = npu_buf_malloc();

        if (!buf) {
            npu_api_print("[Error] malloc failed");
        }
        else {
            int ret;
            int fd;

            struct buf_alloc_req req;
            req.size = size;

            fd = ioctl(npu->fd, NPU_IOCTL_ALLOC_BUFFER, &req);

            if (fd >= 0) {
                buf->fd = fd;
                buf->size = size;
                buf->caddr = (char*)mmap((void*)0,
                                     buf->size,
                                     PROT_READ|PROT_WRITE,
                                     MAP_SHARED,
                                     buf->fd,
                                     0);
            }
            else {
                npu_buf_free(buf);
                buf = NULL;
                npu_api_print("[Error] DMA memory alloc failed");
            }
        }
    }

    npu_api_fout();

    return buf;
}

int buffer_close(npu_buf_t* buf)
{
    enlight_api_ret_t ret;

    npu_api_fin();

    if ((!buf) || (buf->fd < 0)|| (!buf->caddr)) {
        ret = -ENL_API_NOK_INV_ARGS;
        npu_api_print("%p: invalid buf addr", (char*)buf);
    }
    else {
        munmap(buf->caddr, buf->size);
        close(buf->fd);
        npu_buf_free(buf);
        ret = ENL_API_OK;
    }

    npu_api_fout();

    return ret;
}

char* buffer_get_addr(npu_buf_t* buf)
{
    int ret;
    char* addr;

    npu_api_fin();

    if (!buf) {
        addr = NULL;
    }
    else {
        addr = buf->caddr;
    }

    npu_api_fout();

    return addr;
}

int npu_read_ecc_status(
    npu_t*          npu,
    npu_ecc_wdt_status_t* ecc
)
{
	ecc_wdt_access_req_t req;
    enlight_api_ret_t ret;
    int i;

    npu_api_fin();

    if ((!npu) || (npu->fd < 0)) {
        ret = -ENL_API_NOK_INV_ARGS;
    }
    else {
        if (!ioctl(npu->fd, NPU_IOCTL_READ_ECC, &req)) {
	        ecc->wdt_to           = req.wdt_to;
	        ecc->irq_reason       = req.irq_reason;

	        ecc->cbuf.ue_irq_flag = req.cbuf.ue_irq_flag;
	        ecc->cbuf.ce_irq_flag = req.cbuf.ce_irq_flag;
	        ecc->cbuf.ue_cnt      = req.cbuf.ue_cnt;
	        ecc->cbuf.ce_cnt      = req.cbuf.ce_cnt;

            for (i = 0; i < 4; i++) {
	            ecc->gbuf[i].ue_irq_flag = req.gbuf[i].ue_irq_flag;
	            ecc->gbuf[i].ce_irq_flag = req.gbuf[i].ce_irq_flag;
	            ecc->gbuf[i].ue_cnt      = req.gbuf[i].ue_cnt;
	            ecc->gbuf[i].ce_cnt      = req.gbuf[i].ce_cnt;
            }

            for (i = 0; i < 4; i++) {
	            ecc->sram[i].ue_irq_flag = req.sram[i].ue_irq_flag;
	            ecc->sram[i].ce_irq_flag = req.sram[i].ce_irq_flag;
	            ecc->sram[i].ue_status   = req.sram[i].ue_status;
	            ecc->sram[i].ce_status   = req.sram[i].ce_status;
	            ecc->sram[i].ue_cnt      = req.sram[i].ue_cnt;
	            ecc->sram[i].ce_cnt      = req.sram[i].ce_cnt;
	            ecc->sram[i].ce_addr     = req.sram[i].ce_addr;
	            ecc->sram[i].ce_data     = req.sram[i].ce_data;
	            ecc->sram[i].ue_addr     = req.sram[i].ue_addr;
	            ecc->sram[i].ue_data     = req.sram[i].ue_data;
            }
            ret = ENL_API_OK;
        }
        else {
            ret = -ENL_API_NOK_FAIL;
        }
    }

    npu_api_fout();

    return ret;
}
