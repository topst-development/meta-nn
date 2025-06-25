
/**
 *  Openedges Enlight NPU APIs
 *
*/
#ifndef ENLIGHT_NPU_API__
#define ENLIGHT_NPU_API__

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dlfcn.h>

#include "npu.h"

//#define ENLIGHT_API_DEBUG

#ifdef ENLIGHT_API_DEBUG
#   include <stdio.h>
#   define npu_api_fin()                    \
        do {                                \
            (void)fprintf(stderr,           \
                "NPUAPI --> %4d:%s()\n",    \
                    __LINE__,__func__);     \
        } while(0)
#   define npu_api_fout()                   \
        do {                                \
            (void)fprintf(stderr,           \
                "NPUAPI <-- %4d:%s()\n",    \
                    __LINE__,__func__);     \
        } while(0)
#   define npu_api_print(fmt, args...)      \
        do {                                \
            (void)fprintf(stderr,           \
                "NPUAPI %4d:%s:" fmt,       \
                    __LINE__, __func__,     \
                    ##args);                \
        } while(0)
#else
#   define npu_api_fin()                do {} while(0)
#   define npu_api_fout()               do {} while(0)
#   define npu_api_print(fmt, args...)  do {} while(0)
#endif


/** @brief Enum of NPU API return value
 */
typedef enum {
    ENL_API_OK           = 0, /**< return ok                     */
    ENL_API_NOK_INV_ARGS = 1, /**< return nok, invalid input args*/
    ENL_API_NOK_FAIL     = 2, /**< return nok, otherwise         */
} enlight_api_ret_t;


/** @brief Struct of NPU device
 */
typedef struct
{
    int fd;                 /**< file descriptor        */
} npu_t;


/** @brief Struct of NPU DMA buffer
 */
typedef struct
{
    int fd;                 /**<  file descriptor       */
    int size;               /**<  buffer size           */

    char* caddr;            //*<  addr of mmap w/ DMA fd*/
} npu_buf_t;


/** @brief Struct of network
 */
typedef struct
{
    int fd;                 /**<  file descriptor       */

    struct enlight_net* methods;
                            /**<  arributes and methods */
    void* dl;               /**<  dynamic shared object */
} npu_net_t;


/** @brief Struct of performance
 */
typedef struct
{
    int elapsed_in_us;      /**<  inference time in micro-sec */
    int dma;                /**<  ADDR_NPU_PERF_DMA reg value */
    int comp;               /**<  ADDR_NPU_PERF_COMP reg value*/
    int all;                /**<  ADDR_NPU_PERF_ALL reg value */
} npu_perf_t;

/** @brief Struct of ECC and WDT
 */
typedef enum 
{
    ECC_ON        = 0x0,
    ECC_SRAM_OFF  = 0x1,
    ECC_GBUF_OFF  = 0x2,
    ECC_CBUF_OFF  = 0x4,
    ECC_ALL_OFF   = 0x7,
} ecc_enable_t;

typedef struct
{
    ecc_enable_t disable_ue_fail;           /**<  disable run fail by UE
                                                  1: retern success when UE is detected */
    ecc_enable_t disable_ce_fail;           /**<  disable run fail by CE
                                                  1: retern success when CE is detected */
    unsigned int disable_wdt;               /**<  disable WDT timeout error detection   */
} npu_ecc_t;

/** @brief Struct of chip test. Debug Only
 */
typedef struct
{
    unsigned int dev_mlx_bin_idx;           /**<  MLX test bin idx 1~8            */   
    unsigned int dev_wdt_ext_cnt;           /**<  WDT timeout count               */   
    unsigned int dev_wdt_int_cnt;           /**<  WDT rearm   count               */
    unsigned int dev_ecc_test_ctrl;         /**<  [26:18]  err_bit pos1           */
                                            /**<  [16: 8]  err_bit pos0           */
                                            /**<  [ 7: 4]  ld_mask_gbuf           */
    unsigned int dev_mlx_err_inj_mask_data; /**<  MLX err injection mask to data  */   
    unsigned int dev_mlx_err_inj_mask_par;  /**<  MLX err injection mask to parity*/   
} npu_chiptest_t;

/** @brief Struct of ECC WDT status. Debug Only
 */
typedef struct {
	unsigned int ue_irq_flag;
	unsigned int ce_irq_flag;
	unsigned int ue_status;
	unsigned int ce_status;
	unsigned int ue_cnt;
	unsigned int ce_cnt;
	unsigned int ce_addr;
	unsigned int ce_data;
	unsigned int ue_addr;
	unsigned int ue_data;
} npu_ecc_sram_t;

typedef struct {
	unsigned int ue_irq_flag;
	unsigned int ce_irq_flag;
	unsigned int ue_cnt;
	unsigned int ce_cnt;
} npu_ecc_gbuf_t;

typedef struct {
	unsigned int ue_irq_flag;
	unsigned int ce_irq_flag;
	unsigned int ue_cnt;
	unsigned int ce_cnt;
} npu_ecc_cbuf_t;

typedef struct {
	npu_ecc_cbuf_t cbuf;
	npu_ecc_gbuf_t gbuf[MAX_NUM_NPU_CORE];
	npu_ecc_sram_t sram[MAX_NUM_NPU_CORE];
	unsigned int wdt_to;
	unsigned int irq_reason;
} npu_ecc_wdt_status_t;

/** @brief Open NPU device
 *
 *  @param[in]  minor NPU device cluster number. 0 or 1 for minor is allowed
 *  @return     npu   NPU device handle or NULL if failed
 */
npu_t* npu_open(int minor);

/** @brief Reset NPU device
 *
 *  @param[in]  npu  NPU device handle
 *  @param[in]  sw_reset           0: NPU hard reset, 1: NPU soft reset
 *  @param[in]  ecc_pram         ECC, WDT disable param
 *  @param[in]  chiptest_param   chip test param
 *  @return     ret  NPU device reset status. 0 for OK, NOK otherwise
 */
int npu_reset(
    npu_t* npu, 
    int soft_reset,
    npu_ecc_t* ecc_param,
    npu_chiptest_t* chiptest_param
);

/** @brief Close NPU device
 *
 *  @param[in]  npu  NPU device handle
 *  @return     ret  NPU device reset status. 0 for OK, NOK otherwise
 */
int npu_close(npu_t* npu);

/** @brief Write data to NPU device register
 *
 *  @param[in]  npu  NPU device handle
 *  @param[in]  addr write addr
 *  @param[in]  data write data
 *  @return     ret  NPU device write status. 0 for OK, NOK otherwise
 */
int npu_write_reg(
    npu_t*          npu,
    unsigned int    addr,
    unsigned int    data);

/** @brief Read data from NPU device register
 *
 *  @param[in]  npu         NPU device handle
 *  @param[in]  addr        read addr
 *  @param[in]  data        read data
 *  @return     ret         NPU device read status. 0 for OK, NOK otherwise
 */
int npu_read_reg(
    npu_t*          npu,
    unsigned int    addr,
    unsigned int*   data);

/** @brief Read data from NPU device register
 *
 *  @param[in]  npu         NPU device handle
 *  @param[in]  err_status  NPU ECC, WDT status
 *  @return     ret         NPU read status. 0 for OK, NOK otherwise
 */
int npu_read_ecc_status(
    npu_t*              npu,
    npu_ecc_wdt_status_t* err_status
);


// NETWORK
/** @brief Load network to NPU DMA buffer
 *  @param[in]  npu         NPU device handle
 *  @param[in]  sofile      network shared object file name
 *  @param[in]  cmdfile     network command file name
 *  @param[in]  paramfile   network parameter file name
 *  @return     net         loaded network handle, NULL if failed.
 */
npu_net_t* network_load_from_file(
    npu_t*      npu,
    char*       sofile,
    char*       cmdfile,
    char*       paramfile);

/** @brief Load network cmd, param to NPU DMA buffer
 *
 *  @param[in]  npu         NPU device handle
 *  @param[in]  cmd         npu cmd buffer base
 *  @param[in]  cmd_size    npu cmd buffer size
 *  @param[in]  param       npu parameter buffer base
 *  @param[in]  param_size  npu parameter buffer size
 *  @return     net         loaded network handle or NULL if failed.
 */
npu_net_t* network_load(
    npu_t*      npu,
    char*       cmd,
    int         cmd_size,
    char*       param,
    int         param_size);

/** @brief Set input image color format
 *
 *  @param[in]  net   network handle
 *  @param[in]  color color format, NPU_COLOR_RGB, NPU_COLOR_YUV
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_set_color_format(
    npu_net_t*  net,
    int         color_format);

/** @brief Run NPU to do inference and wait until inference done
 *
 *  @param[in]  net   network handle
 *  @param[in]  in    input image buffer handle
 *  @param[in]  out   output buffer handle
 *  @param[in]  timeout_in_ms
 *                    inference time out in ms
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_run(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    int         timeout_in_ms);

/** @brief  Run NPU to do inference
 *
 *  @param[in]  net   network handle
 *  @param[in]  in    input image buffer handle
 *  @param[in]  out   output buffer handle
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_issue_run(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out);

/** @brief  Wait for inference done
 *
 *  @param[in]  net   network handle
 *  @param[in]  timeout_in_ms
 *                    inference time out in ms
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_wait_done(
    npu_net_t*  net,
    int         timeout_in_ms);

/** @brief Run NPU to do inference for performance profiling
 *
 *  @param[in]  net   network handle
 *  @param[in]  in    input image buffer handle
 *  @param[in]  out   output buffer handle
 *  @param[in]  perf  performance report buffer handle
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_profile(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    npu_perf_t* perf);

/** @brief Get network input image buffer size
 *
 *  @param[in]  net   network handle
 *  @return     ret   valid if ret > 0,
 *                    error if ret <= 0.
 */
int network_get_input_size(npu_net_t* net);

/** @brief Get network output buffer size
 *
 *  @param[in]  net   network handle
 *  @return     ret   valid if ret > 0,
 *                    error if ret <= 0.
 */
int network_get_output_size(npu_net_t* net);

/** @brief Get network input image width
 *
 *  @param[in]  net   network handle
 *  @return     ret   valid if ret > 0,
 *                    error if ret <= 0.
 */
int network_get_input_width(npu_net_t* net);

/** @brief Get network input image height
 *
 *  @param[in]  net   network handle
 *  @return     ret   valid if ret > 0,
 *                    error if ret <= 0.
 */
int network_get_input_height(npu_net_t* net);

/** @brief Get network post-process type
 *
 *          post-process types = {
 *              ENLIGHT_POST_NONE       = 0
 *              ENLIGHT_POST_CLASSIFIER = 1
 *              ENLIGHT_POST_DETECTOR   = 2
 *              ENLIGHT_POST_CUSTOM     = 3
 *          }
 *  @param[in]  net   network handle
 *  @return     ret   valid if ret > 0,
 *                    error if ret <= 0.
 */
int network_get_type(npu_net_t* net);

/** @brief Run network post-process
 *
 *  @param[in]  net   network handle
 *  @param[in]  out   output buffer handle
 *  @param[out] result   post-process output
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_run_postprocess(
    npu_net_t*  net,
    npu_buf_t*  out,
    void*       result);

/** @brief Close network
 *
 *  @param[in]  net   network handle
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_close(npu_net_t* net);


// BUFFER
/** @brief Alloc NPU buffer
 *
 *  @param[in]  npu  NPU device handle
 *  @param[in]  size buffer size
 *
 *  @return     NPU buffer handle
 */
npu_buf_t* buffer_alloc(npu_t* npu, int size);

/** @brief Release NPU buffer
 *
 *  @param[in]  npu  NPU device handle
 *
 *  @return     close status. 0 for OK, NOK otherwise
 */
int buffer_close(npu_buf_t* buf);

/** @brief Get address of NPU buffer
 *
 *  @param[in]  npu  NPU buffer handle
 *
 *  @return     NPU buffer addr, or NULL in case of error
 */
char* buffer_get_addr(npu_buf_t* buf);

/*
    npu_api_mem.c
*/

/** @brief NPU API memory allocation
 *
 *  @param[in]  size  buffer handle
 *  @return     addr  memory base addr
 */
char* npu_api_malloc(int size);

/** @brief NPU API memory free
 *
 *  @param[in]  addr  memory base addr
 */
void npu_api_free(void *addr);

/** @brief Memory allocation of npu_buf_t
 *
 *  @return     addr  memory base addr
 */
npu_buf_t* npu_buf_malloc(void);

/** @brief Memmory free of npu_buf_t
 *
 *  @param[in]  addr  memory base addr
 */
void npu_buf_free(void* addr);

/** @brief Memory allocation of npu_net_t
 *
 *  @return     addr  memory base addr
 */
npu_net_t* npu_net_malloc(void);

/** @brief Memmory free of npu_net_t
 *
 *  @param[in]  addr  memory base addr
 */
void npu_net_free(void *addr);

/** @brief Memory allocation of struct enlight_net
 *
 *  @return     addr  memory base addr
 */
struct enlight_net* enlight_net_malloc(void);

/** @brief Memmory free of struct enlight_net
 *
 *  @param[in]  addr  memory base addr
 */
void enlight_net_free(void *addr);

/** @brief Struct of single object
 */
typedef struct {
    float           x_min;      /**< object box position        */
    float           x_max;      /**< object box position        */
    float           y_min;      /**< object box position        */
    float           y_max;      /**< object box position        */
    int16_t         cls;        /**< object class               */
    float           score;      /**< object confidence score    */
    int16_t         img_w;      /**< input image width          */
    int16_t         img_h;      /**< input image height         */
} enlight_obj_t;

/** @brief Struct of detected objects,
 *              output of object detection network's post-process
 */
typedef struct {
    int cnt;                    /**< number of detected objects */
    enlight_obj_t   obj[256];   /**< MAX_DETECT_BOX=256         */
                                /**< detected objs description  */
} enlight_objs_t;


/** @brief Struct of classification
 *              output of classificatton network's post-process
 */
typedef struct {
    int class_ids[8];           /**< MAX_BATCH_NUM=8            */
                                /**< detected cls description   */
} enlight_batch_cls_t;

/** @brief Struct of custom network output
 *      FIXME.
 *      the struct of custom network output should be modified
 *      according to custome network post processing
 */
typedef struct {
    int output_data;
} enlight_custom_t;

/** @brief Enum of logistic type
 */
typedef enum {
    ENLIGHT_POST_NONE,          /**< post-process : not defined */
    ENLIGHT_POST_CLASSIFIER,    /**< post-process : classifier  */
    ENLIGHT_POST_DETECTOR,      /**< post-process : detector    */
    ENLIGHT_POST_CUSTOM,        /**< post-process : unknwon     */
} enlight_postproc_t;

/** @brief Struct of network
 */
struct enlight_net {
    int         fd;
    int         cmd_size;       /**< command buffer size        */
    int         weight_size;    /**< weight buffer size         */
    int         work_size;      /**< work buffer size           */
    int         input_size;     /**< input buffer size          */
    int         output_size;    /**< output buffer size         */
    int         post_type;      /**< post-process type          */
    int*        img_size;       /**< input dimension            */
    long long   conv_mac_num;   /**< convolution MAC number     */
    int         batch_size;     /**< batch size                 */
    char        name[128];      /**< network name               */

    int         (*get_buffer_size)(int idx);
    int*        (*get_image_size)(void);
    char*       (*get_network_name)(void);
    int         (*get_post_type)(void);
    int         (*run_post)(void* output_base,
                            int batch_size,
                            void* post_output);

    int         (*get_output_tensor_num)(void);
    int         (*get_output_tensor)(int idx);
    int         (*get_tensor_size)(int idx);
    int         (*get_tensor_offset)(int idx);
    char*       (*get_tensor_name)(int idx);
};

#endif
