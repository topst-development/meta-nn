
#ifndef __ENLIGHT_NETWORK_H__
#define __ENLIGHT_NETWORK_H__
/*
    Openedges Enlight Network Compiler

    Network struct and APIs.
*/

#include "enlight_data_type.h"

#define enlight_network_err(...)  do {printf(__VA_ARGS__); } while(0)
#define enlight_network_log(...)  do {printf(__VA_ARGS__); } while(0)

#ifdef ENLIGHT_NETWORK_DEBUG
#   define enlight_network_dbg(...)  do {printf(__VA_ARGS__); } while(0)
#else
#   define enlight_network_dbg(...)  do {} while(0)
#endif


/** @brief Return network name.
 *
 *      Network name,  file name of quantized enlight.
 *
 *  @param instance Network instance.
 *  @return         String of network name.
 */
char* enlight_get_name(enlight_network_t *inst);

/** @brief Return post-process type
 *
 *     post-process type 
 *          ENLIGHT_POST_DETECTOR
 *          ENLIGHT_POST_CLASSIFIER
 *       or ENLIGHT_POST_NONE
 *
 *  @param instance Network instance.
 *  @return         post-process type
 */
enlight_postproc_t
        enlight_get_post_type(enlight_network_t *inst);

/** @brief Return input image C, H, W
 *
 *  @param instance Network instance.
 *  @return         img_sizes address
 */
int*    enlight_get_img_sizes(enlight_network_t *inst);

/** @brief Return output tensors number
 *
 *  @param instance Network instance.
 *  @return         output tensor nubmer
 */
int     enlight_get_output_tensor_num(enlight_network_t *inst); 

/** @brief Return output tensors address
 *
 *  @param instance Network instance.
 *  @return         addr of output_tensors[idx]
 */
enlight_act_tensor_t *
        enlight_get_output_tensor(enlight_network_t *inst, int idx); 

/** @brief Return tensor dims N, H,  W,  C
 *
 *  @param tensor      tensor
 *  @return            tensor dimension N, H, W, C
 */
void    enlight_get_tensor_dimensions(enlight_act_tensor_t *t, int dims[4]);

/** @brief Return tensor size
 *
 *  @param tensor      tensor
 *  @return            tensor size = (1 * H * W * aligend(C, 32))
 */
int     enlight_get_tensor_size(enlight_act_tensor_t *t);

/** @brief Return tensor buffer addr
 *
 *  @param tensor      tensor
 *  @return            tensor addr = output buffer base + off
 */
void*   enlight_get_tensor_addr(enlight_act_tensor_t *t);

/** @brief Return tensor buffer offset from output buffer base
 *
 *  @param tensor      tensor
 *  @return            tensor offset
 */
int     enlight_get_tensor_offset(enlight_act_tensor_t *t);

/** @brief Return int8_t quantized tensor data w/ dims=[H, W, C]
 *
 *      Read quantized output data from source tensor
 *      and write it to target buffer.
 *      
 *      To allocate/free target buffer by Caller.
 *
 *  @param tensor      source tensor
 *  @param quant_data  target buffer
 */
void    enlight_get_tensor_qdata(enlight_act_tensor_t *t, int8_t *quant_data);

/** @brief Return float tensor data w/ dims=[H, W, C]
 *
 *      Read quantized output data from source tensor,
 *      De-quantize and then write it to target buffer.
 *      
 *      To allocate/free target buffer by Caller.
 *
 *  @param tensor      source tensor
 *  @param quant_data  target buffer
 */
void    enlight_get_tensor_data(enlight_act_tensor_t *t, float *de_quant_data);

/** @brief Return float tensor data w/ dims=[H, W, C]
 *  @param tensor      source tensor
 *  @return            float tensor's scale
 */
float   enlight_get_tensor_scale(enlight_act_tensor_t *t);

/** @brief Return sigle int8_t quantized tensor data
 *
 *      Return quantized_data[y_off, x_off, c_off]
 *
 *  @param tensor      source tensor
 *  @param y_off       offset in height
 *  @param x_off       offset in width
 *  @param c_off       offset in channel
 *  @return            quantized data        
 */
int8_t  enlight_get_tensor_qdata_by_off(enlight_act_tensor_t *t, int y_off, int x_off, int c_off);

/** @brief Return single float a tensor data
 *
 *      Return quantized_data[y_off, x_off, c_off] / scale
 *
 *  @param tensor      source tensor
 *  @param y_off       offset in height
 *  @param x_off       offset in width
 *  @param c_off       offset in channel
 *  @return            de-quantized data        
 */
float   enlight_get_tensor_data_by_off(enlight_act_tensor_t *t, int y_off, int x_off, int c_off);

/** @brief Return tensor data type
 *
 *  @param tensor      tensor
 *  @return data type in enlight_data_t
 */
int enlight_get_tensor_quant_data_type(enlight_act_tensor_t *t);

/** @brief Return code/weight/input/output/work buffer size
 *
 *  @param instance Network instance.
 *  @param buf_type enlight_buf_base_t type for 
                    code, weight, work, input, or output buffer
 *  @return         byte size
 */
int enlight_get_buf_size(enlight_network_t *inst, enlight_buf_base_t buf_type);

#ifdef BARE_METAL_FW_DEV
// APIs only for ENLIGHT toolkit developer
uint8_t* enlight_get_code_base(enlight_network_t *inst);
#endif


/*************************************************************************/
/* SSD post process extension APIs                                       */
/*************************************************************************/

/** @brief Return float type value of Confidence threshold
 *
 *  @param  param ssd_detector param address
 *  @return       float type value
 */
float enlight_ssd_get_thres_conf(enlight_ssd_detector_t* param);

/** @brief Return float type value of IOU threshold
 *
 *  @param  param ssd_detector param address
 *  @return       float type value
 */
float enlight_ssd_get_thres_iou(enlight_ssd_detector_t* param);

/** @brief Return class number
 *
 *  @param  param ssd_detector param address
 *  @return       class number
 */
int enlight_ssd_get_num_class(enlight_ssd_detector_t* param);

/** @brief Return box number
 *
 *  @param  param ssd_detector param address
 *  @return       box number
 */
int enlight_ssd_get_num_box(enlight_ssd_detector_t* param);

/** @brief Return default box scale_1024
 *
 *  @param  param ssd_detector param address
 *  @return       scale
 */
float enlight_ssd_get_default_box_scale(enlight_ssd_detector_t* ssd_param);

/** @brief Return default box buffer addr
 *
 *  @param  param ssd_detector param address
 *  @return       buffer addr
 */
uint8_t* enlight_ssd_get_default_box_buf_addr(enlight_ssd_detector_t* param);

/** @brief Return default box address
 *      Process as followings steps.
 *          0. Allocate buffer, float buf[num_box];
 *          1. buf[i] = (float)default_box[i] / (1 << default_box_scale);
 *
 *      Alloc/free buffer by Caller
 *
 *  @param  param ssd_detector param address
 *  @param  buf   float type buffer
 */
void      enlight_ssd_get_float_default_boxes(enlight_ssd_detector_t* param, float* buf);

/** @brief Return conf tensor scale
 *
 *  @param  param ssd_detector param address
 *  @param  idx   conf tensor index
 */
float     enlight_ssd_get_conf_scales(enlight_ssd_detector_t* param, int idx);

/** @brief Return loc tensor scale
 *
 *  @param  param ssd_detector param address
 *  @param  idx   loc tensor index
 */
float     enlight_ssd_get_loc_scales(enlight_ssd_detector_t* param, int idx);

/** @brief Return conf sigmoid/softmax look-up table address
 *
 *  @param  param ssd_detector param address
 *  @param  idx   conf tensor index
 */
uint32_t* enlight_ssd_get_conf_logistic_table(enlight_ssd_detector_t* param, int idx);

/** @brief Return loc exponent look-up table address
 *
 *  @param  param ssd_detector param address
 *  @param  idx   loc tensor index
 */
uint32_t* enlight_ssd_get_loc_exp_table(enlight_ssd_detector_t* param, int idx);

/** @brief Return detected objects result address.
 *
 *          Detected objects of final output of SSD post-process
 *
 *  @param  param ssd_detector param address
 *  @param  idx   loc tensor index
 */
enlight_objs_t*   enlight_ssd_get_objs_buf_base(enlight_ssd_detector_t* param);

/** @brief Return loc tensors addr
 *
 *  @param  param ssd_detector param address
 *  @param  tensor_buf  tensor base return buffer
 *  @return       tensors number
 */
int enlight_ssd_get_loc_tensors(enlight_ssd_detector_t* param, enlight_act_tensor_t ***tensor_buf);

/** @brief Return conf tensors addr
 *
 *  @param  param ssd_detector param address
 *  @param  tensor_buf  tensor base return buffer
 *  @return       tensors number
 */
int enlight_ssd_get_conf_tensors(enlight_ssd_detector_t* param, enlight_act_tensor_t ***tensor_buf);

/** @brief Return Is background class in classes
 *
 *  @param  param ssd_detector param address
 *  @return       background class in classes
 */
int enlight_ssd_get_bg_in_cls(enlight_ssd_detector_t* param);

/** @brief Return logistic type
 *
 *  @param  param ssd_detector param address
 *  @return       logistic type
 */
enlight_logistic_t enlight_ssd_get_logistic(enlight_ssd_detector_t* param);

/** @brief Return nms method type
 *
 *  @param  param ssd_detector param address
 *  @return       nms method type
 */
enlight_nms_t enlight_ssd_get_nms_method(enlight_ssd_detector_t* param);


/*************************************************************************/
/* YOLO post process extension APIs                                      */
/*************************************************************************/
int       enlight_yolo_get_softmax_use(enlight_yolo_detector_t* p);
int       enlight_yolo_get_loc_tensors(enlight_yolo_detector_t* p, enlight_act_tensor_t ***tensor_buf);
int       enlight_yolo_get_conf_tensors(enlight_yolo_detector_t* p, enlight_act_tensor_t ***tensor_buf);
float     enlight_yolo_get_thres_conf(enlight_yolo_detector_t* p);
float     enlight_yolo_get_thres_iou(enlight_yolo_detector_t* p);
int       enlight_yolo_get_num_class(enlight_yolo_detector_t* p);
int*      enlight_yolo_get_num_grids(enlight_yolo_detector_t* p);
int       enlight_yolo_get_num_anchor(enlight_yolo_detector_t* p);
float     enlight_yolo_get_default_box_scale(enlight_yolo_detector_t* p);
uint8_t*  enlight_yolo_get_default_box_buf_addr(enlight_yolo_detector_t* p);
void      enlight_yolo_get_float_default_boxes(enlight_yolo_detector_t* p, float* buf);
float     enlight_yolo_get_output_scales(enlight_yolo_detector_t* p, int output_index);
uint32_t* enlight_yolo_get_output_exp_table(enlight_yolo_detector_t* p, int output_index);
uint32_t* enlight_yolo_get_output_sig_table(enlight_yolo_detector_t* p, int output_index);
enlight_objs_t*   enlight_yolo_get_objs_buf_base(enlight_yolo_detector_t* p);

/** @brief Return whether Yolo outputs is divided to conf, loc or not
 *
 *  @param  param       yolo_detector parm address
 *  @return       Return 1 if outputs of Yolo network is divided to conf, loc else 0
 */
int       enlight_yolo_get_out_tensor_divided(enlight_yolo_detector_t* param);

/** @brief Return whether sigmoid has been applied to Yolo Network outputs
 *
 *  @param  param       yolo_detector instance address
 *  @return       Return 1 if sigmoid has been applied to Yolo Network outputs else 0
 */
int       enlight_yolo_get_sigmoid_applied(enlight_yolo_detector_t* param);

/*************************************************************************/
/* Classification post process extension APIs                                      */
/*************************************************************************/
int       enlight_class_get_output_tensors(enlight_classification_t* p, enlight_act_tensor_t ***tensor_buf);
int       enlight_class_get_num_class(enlight_classification_t* p);

#endif /*__ENLIGHT_NETWORK__*/
