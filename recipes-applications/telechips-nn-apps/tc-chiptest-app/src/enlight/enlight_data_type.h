
#ifndef __ENLIGHT_DATA_TYPE_H__
#define __ENLIGHT_DATA_TYPE_H__

/**
 *  Openedges Enlight Network Compiler
 *
 *  Enlight Data types definition
*/

#include <stdio.h>
#include <stdint.h>

#define MAX_NUM_OUTPUT          32      /**< Max number of output layers    */
#define MAX_NUM_LOC             8       /**< Max number of ssd loc layers   */
#define MAX_NUM_CONF            8       /**< Max number of ssd conf layers  */
#define MAX_DETECT_BOX          256     /**< max detected box num           */
#define TENSOR_NAME_LEN         33      /**< Max number of output layers    */


/** @brief Enum of logistic type
 */
typedef enum {
    LOGISTIC_NONE,                      /**< confidence logistic: none      */
    LOGISTIC_SIGMOID,                   /**< confidence logistic: sigmoid   */
    LOGISTIC_SOFTMAX,                   /**< confidence logistic: softmax   */
} enlight_logistic_t;


/** @brief Enum of nms method type
 */
typedef enum {
    NMS_DEFAULT,                        /**< nms type of detector: normal   */
    NMS_FAST,                           /**< nms type of detector: fast     */
} enlight_nms_t;


/** @brief Enum of Enlight buffer type
 */
typedef enum {
    ENLIGHT_BUF_CODE    = 0,            /**< inference buffer type: code    */
    ENLIGHT_BUF_WGHT    = 1,            /**< inference buffer type: weight  */
    ENLIGHT_BUF_WORK    = 2,            /**< inference buffer type: work    */
    ENLIGHT_BUF_INPUT   = 3,            /**< inference buffer type: input   */
    ENLIGHT_BUF_OUTPUT  = 4,            /**< inference buffer type: output  */
    ENLIGHT_BUF_NUM     = 8,            /**< max number of buffer           */
} enlight_buf_base_t;


/** @brief Struturce of Enlight data type
 */
typedef enum {
    ENLIGHT_DTYPE_UINT4,                /**< unsigned integer 4 bits        */
    ENLIGHT_DTYPE_INT4,                 /**< signed integer 4 bits          */
    ENLIGHT_DTYPE_UINT8,                /**< unsigned integer 8 bits        */
    ENLIGHT_DTYPE_INT8,                 /**< signed integer 8 bits          */
    ENLIGHT_DTYPE_UINT16,               /**< unsigned integer 16 bits       */
    ENLIGHT_DTYPE_INT16,                /**< signed integer 16 bits         */
    ENLIGHT_DTYPE_UINT32,               /**< unsigned integer 32 bits       */
    ENLIGHT_DTYPE_INT32,                /**< signed integer 32 bits         */
    ENLIGHT_DTYPE_FP16,                 /**< float 16 bits                  */
    ENLIGHT_DTYPE_FP32,                 /**< float 32 bits                  */
    ENLIGHT_DTYPE_FLOAT,                /**< float                          */
} enlight_data_t;


/** @brief Struturce to represent weight tensor of layer
 */
typedef struct {
    int     data_type;                  /**< data type of weight and bias   */
    void*   wbuf;                       /**< base address of weight         */
    int     wsize;                      /**< size of weight                 */
    int     fb;                         /**< addr buffer index of weight    */
    void*   bbuf;                       /**< base address of bias           */
    int     bsize;                      /**< size of bias                   */
} enlight_weight_tensor_t;


/** @brief Struturce to represent output/activation tensor of layer
 *  How to get float type output
 *      val = (float)(buf[i] + zp) / scale
 */
typedef struct {
    int         data_type;              /**< data type of tensor            */
    void*       base;                   /**< base address of tensor         */
    int         buf;                    /**< offset of tensor               */
    int         size;                   /**< size of tensor                 */
    int         fb;                     /**< addr buffer index of tensor    */
    int         num_dimension;          /**< number of dimension            */
    int         dimensions[4];          /**< dimensions of tensor           */
    float       scale;                  /**< float scale                    */
    int         zp;                     /**< zero point                     */
    int         output;                 /**< output flag.                   */
    char        name[TENSOR_NAME_LEN];  /**< tensor name                    */
} enlight_act_tensor_t;


/** @brief Struturce of external buffer sizes.
 */
typedef struct {
    int         code_buf_size;          /**< npu code buf size              */
    int         wght_buf_size;          /**< quantized weight buffer size   */
    int         input_buf_size;         /**< input buffer size              */
    int         output_buf_size;        /**< output buffer size             */
    int         work_buf_size;          /**< temporal buffer size           */
} enlight_buf_size_t;


/** @brief Description of a detected object.
 */
typedef struct {
    int16_t     x_min;                  /**< object box position            */
    int16_t     x_max;                  /**< object box position            */
    int16_t     y_min;                  /**< object box position            */
    int16_t     y_max;                  /**< object box position            */
    int16_t     cls;                    /**< object class                   */
    int16_t     score;                  /**< object confidence score        */
    int16_t     img_w;                  /**< input image width              */
    int16_t     img_h;                  /**< input image height             */
} enlight_obj_t;


/** @brief All detected objects.
 */
typedef struct {
    int         cnt;                            /**< number of detected objects     */
    enlight_obj_t
                obj[MAX_DETECT_BOX];            /**< detected objects description   */
} enlight_objs_t;


/** @brief Struturce of SSD post-process
 */
typedef struct {
    float       th_conf;                        /**< conf_thresold            */
    float       th_iou;                         /**< iou_thresold             */
    int         num_class;                      /**< number of class          */
    int         num_box;                        /**< number of default box    */
    int         num_anchor;                     /**< anchor number per a grid */
    float       default_box_scale;              /**< float box_val[i] = 
                                                     default_box[i] / default_box_scale */
    uint8_t*    default_box;                    /**< default box address      */
    uint32_t*   loc_exp_tables;                 /**< loc exp tables           */
    uint32_t*   conf_logistic_tables;           /**< confidence exp tables    */
    int         loc_tensors_num;                /**< loc tensor num           */
    int         conf_tensors_num;               /**< loc tensor num           */
    int         bg_in_cls;                      /**< backgroud existence in class */

    enlight_logistic_t    
                logistic;                       /**< logistic type            */
    enlight_nms_t         
                nms_method;                     /**< nms method type          */
    enlight_act_tensor_t* 
                loc_tensors[MAX_NUM_LOC];
                                                /**< loc tensors              */
    enlight_act_tensor_t* 
                conf_tensors[MAX_NUM_CONF];
                                                /**< conf tensors             */
    enlight_objs_t        
                detected_objects;               /**< SSD detected objects     */
} enlight_ssd_detector_t;


/** @brief Struturce of YOLO post-process
 */
typedef struct {
    int         softmax_use;                    /**< softmax used in Yolov2     */
    int         sigmoid_applied;                /**< sigmoid applied to output  */
    float       th_conf;                        /**< conf_thresold              */
    float       th_iou;                         /**< iou_thresold               */
    int         num_class;                      /**< number of class            */
    int         num_grids[3*2];                 /**< grid num for 3 stages      */
    int         num_anchor;                     /**< anchor num                 */
    float       default_box_scale;              /**< float box_val[i] = 
                                                  default_box[i] / default_box_scale */
    uint8_t*    default_box;                    /**< default box address        */
    float       loc_scales[MAX_NUM_OUTPUT];     /**< loc scales                 */
    float       conf_scales[MAX_NUM_OUTPUT];    /**< conf scales                */
    uint32_t*   exp_tables;                     /**< exp tables                 */
    uint32_t*   sig_tables;                     /**< sigmoid tables             */
    int         loc_tensors_num;                /**< loc tensor num             */
    int         conf_tensors_num;               /**< conf tensor num            */
    int         out_tensor_divided;             /**< flag output tensor divided */

    enlight_act_tensor_t* 
                output_tensors[MAX_NUM_OUTPUT]; /**< output tensors             */
    enlight_act_tensor_t* 
                loc_tensors[MAX_NUM_LOC];       /**< loc tensors                */
    enlight_act_tensor_t* 
                conf_tensors[MAX_NUM_CONF];     /**< conf tensors               */

    enlight_objs_t        
                detected_objects;               /**< YOLO detected objects      */
} enlight_yolo_detector_t;


/** @brief Struturce of Classification post-process
 */
typedef struct {
    int         num_class;                      /**< number of class            */
    enlight_act_tensor_t* 
                output_tensors[MAX_NUM_OUTPUT]; /**< output tensors             */
    int         output_tensors_num;             /**< output tensor num          */
} enlight_classification_t;

/** @brief Enum of logistic type
 */
typedef enum {
    ENLIGHT_POST_NONE,                          /**< post-process : not defined */
    ENLIGHT_POST_CLASSIFIER,                    /**< post-process : classifier  */
    ENLIGHT_POST_DETECTOR,                      /**< post-process : detector    */
    ENLIGHT_POST_CUSTOM,                        /**< post-process : unknwon     */
} enlight_postproc_t;

/** @brief Struturce of complied network
 */
typedef struct {
    char*       network_name;                   /**< Network name               */
    enlight_postproc_t
                post_proc;                      /**< post-process type          */
                
    int         img_sizes[3];                   /**< input image size C, H, W   */
    enlight_buf_size_t*     
                buf_sizes;                      /**< external memory size info  */
    void*       post_proc_extension;            /**< post-process extension of 
                                                     enlight_ssd_detector_t
                                                     enlight_yolo_detector_t or
                                                     enlight_classification_t   */
    int         output_tensors_num;             /**< output tensors number      */
    enlight_act_tensor_t*   
                output_tensors[MAX_NUM_OUTPUT]; /**< output tensors             */

                                                /**< Only for debug             */
    int         all_tensors_num;                /**< all output tensors number  */
    enlight_act_tensor_t**  
                all_tensors;                    /**< all tensors output table   */

                                                /**< Only For toolkit developer */
#ifdef BARE_METAL_FW_DEV
    uint32_t*   npu_cmd_codes;                  /**< NPU command code words     */
#endif
} enlight_network_t;


#endif /*__ENLIGHT_DATA_TYPE_H__*/
