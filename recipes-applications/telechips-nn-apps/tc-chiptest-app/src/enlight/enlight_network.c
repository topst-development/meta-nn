/*
    Openedges Enlight Network Compiler

    Network APIs functions.
*/

#include "enlight_network.h"


char*   enlight_get_name(enlight_network_t *inst) 
{
    return inst->network_name;
}

enlight_postproc_t
        enlight_get_post_type(enlight_network_t *inst) 
{
    return inst->post_proc;
}

int*    enlight_get_img_sizes(enlight_network_t *inst) 
{
    return inst->img_sizes;
}

int     enlight_get_output_tensor_num(enlight_network_t *inst) 
{
    return inst->output_tensors_num;
}

enlight_act_tensor_t *
        enlight_get_output_tensor(enlight_network_t *inst, int idx) 
{
    return inst->output_tensors[idx];
}

void    enlight_get_tensor_dimensions(enlight_act_tensor_t *t, int dims[4])
{
    dims[0] = t->dimensions[0];
    dims[1] = t->dimensions[1];
    dims[2] = t->dimensions[2];
    dims[3] = t->dimensions[3];
}

int     enlight_get_tensor_size(enlight_act_tensor_t *t)
{
    return t->size;
}

void*   enlight_get_tensor_addr(enlight_act_tensor_t *t)
{
    return (void*)(t->base + t->buf);
}

int     enlight_get_tensor_offset(enlight_act_tensor_t *t)
{
    return t->buf;
}

void    enlight_get_tensor_qdata(enlight_act_tensor_t *t, int8_t *dst)
{
    int i;

    enlight_network_dbg("tensor %08x\n", t);
    enlight_network_dbg("base   %08x\n", t->base);
    enlight_network_dbg("off    %08x\n", t->buf);

    int8_t* src = (int8_t*)(t->base + (unsigned long)t->buf);

    enlight_network_dbg("src %08x\n", src);

    if (t->output) {
        for (i = 0 ; i < t->size; i++)
            *dst++ = (int8_t)src[i];
    }
    else {
        const int WORD_SIZE = 32; 

        int ii, jj, kk;
        int c, h, w;

        c = t->dimensions[1];
        w = t->dimensions[2];
        h = t->dimensions[3];

        for(ii = 0; ii < h; ii++) {
            for(jj = 0; jj < w; jj++)
                for(kk = 0; kk < c; kk++) {

                    i = (kk % WORD_SIZE);
                    i += (WORD_SIZE * jj);
                    i += (WORD_SIZE * w * ii);
                    i += ((kk / WORD_SIZE) * h * w); 

                    *dst++ = (int8_t)src[i];
                }
        }
    }
}

void    enlight_get_tensor_data(enlight_act_tensor_t *t, float *dst)
{
    int i;

    enlight_network_dbg("tensor %08x\n", t);
    enlight_network_dbg("base   %08x\n", t->base);
    enlight_network_dbg("off    %08x\n", t->buf);

    int8_t* src = (int8_t*)(t->base + (unsigned long)t->buf);

    enlight_network_dbg("src %08x\n", src);

    if (t->output) {
        for (i = 0 ; i < t->size; i++)
            *dst++ = (float)src[i] / t->scale;
    }
    else {
        const int WORD_SIZE = 32; 

        int ii, jj, kk;
        int c, h, w;

        c = t->dimensions[1];
        w = t->dimensions[2];
        h = t->dimensions[3];

        for(ii = 0; ii < h; ii++) {
            for(jj = 0; jj < w; jj++)
                for(kk = 0; kk < c; kk++) {

                    i = (kk % WORD_SIZE);
                    i += (WORD_SIZE * jj);
                    i += (WORD_SIZE * w * ii);
                    i += ((kk / WORD_SIZE) * h * w); 

                    *dst++ = (float)src[i] / t->scale;
                }
        }
    }
}

float   enlight_get_tensor_scale(enlight_act_tensor_t *t)
{
    return t->scale;
}

int8_t  enlight_get_tensor_qdata_by_off(enlight_act_tensor_t *t, int y_off, int x_off, int c_off)
{
    const int WORD_SIZE = 32; 
    int idx;
    int8_t* src = (int8_t*)(t->base + (unsigned long)t->buf);

    int* dims = t->dimensions;

    int c = (dims[1] + (WORD_SIZE - 1)) & 0xFFE0;
    int h = dims[2];
    int w = dims[3];

    if (t->output) {
        idx = c_off + (c * x_off) + (c * w * y_off); 
    }
    else {
        idx = (c_off % WORD_SIZE) + (WORD_SIZE * x_off) + (WORD_SIZE * w * y_off) + ((c_off / WORD_SIZE) * h * w); 
    }

    enlight_network_dbg("output %3d\n", t->output);
    enlight_network_dbg("dims %3d %3d %3d %3d\n", dims[0], dims[1], dims[2], dims[3]);
    enlight_network_dbg("hwc  %3d %3d %3d\n", h, w, c);
    enlight_network_dbg("off y x c  %3d %3d %3d\n", y_off, x_off, c_off);
    enlight_network_dbg("idx %d\n", idx);
    enlight_network_dbg("base offset 0x%08x + 0x%08x = 0x%08x\n", src, (unsigned long)tensor->buf, &src[idx]);

    return src[idx];
}

float enlight_get_tensor_data_by_off(enlight_act_tensor_t *tensor, int y_off, int x_off, int c_off)
{
    float src;
    src = (float)enlight_get_tensor_qdata_by_off(tensor, y_off, x_off, c_off);
    return src / tensor->scale;
}

int enlight_get_tensor_quant_data_type(enlight_act_tensor_t *tensor)
{
    return tensor->data_type;
}

int enlight_get_buf_size(enlight_network_t *inst, enlight_buf_base_t buf_type)
{
    if (buf_type == ENLIGHT_BUF_CODE)
        return inst->buf_sizes->code_buf_size;
    else if (buf_type == ENLIGHT_BUF_WGHT)
        return inst->buf_sizes->wght_buf_size;
    else if (buf_type == ENLIGHT_BUF_WORK)
        return inst->buf_sizes->work_buf_size;
    else if (buf_type == ENLIGHT_BUF_INPUT)
        return inst->buf_sizes->input_buf_size;
    else if (buf_type == ENLIGHT_BUF_OUTPUT)
        return inst->buf_sizes->output_buf_size;
    else
        enlight_network_err("wrong buf type:%d\n", buf_type);
        return 0;
}

#ifdef BARE_METAL_FW_DEV
// APIs only for ENLIGHT toolkit developer
uint8_t* enlight_get_code_base(enlight_network_t *inst)
{
    return (uint8_t*)inst->npu_cmd_codes;
}
#endif

/******************************************************************************/
/*  SSD post process extension APIs                                           */
/******************************************************************************/
float enlight_ssd_get_thres_conf(enlight_ssd_detector_t* param) 
{
    return param->th_conf;
}

float enlight_ssd_get_thres_iou(enlight_ssd_detector_t* param) 
{
    return param->th_iou;
}

int enlight_ssd_get_num_class(enlight_ssd_detector_t* param) 
{
    return param->num_class;
}

int enlight_ssd_get_num_box(enlight_ssd_detector_t* param) 
{
    return param->num_box;
}

float enlight_ssd_get_default_box_scale(enlight_ssd_detector_t* param)
{
    return param->default_box_scale;
}

uint8_t* enlight_ssd_get_default_box_buf_addr(enlight_ssd_detector_t* param)
{
    return param->default_box;
}

void enlight_ssd_get_float_default_boxes(enlight_ssd_detector_t* param, float* buf)
{
    int i;
    int num_box = param->num_box;
    float scale = param->default_box_scale;
    uint8_t* default_box = param->default_box;

    for (i=0; i < num_box; i++)
    {
        buf[i*4] = ((float)default_box[i*4]) / scale;
    }
}

float enlight_ssd_get_conf_scales(enlight_ssd_detector_t* param, int idx)
{
    if (idx >= param->conf_tensors_num) {
        enlight_network_err("Wrong conf idx %d\n", idx);
        return 0;
    }

    enlight_act_tensor_t* out_tensor = param->conf_tensors[idx];
    return out_tensor->scale;
}

float enlight_ssd_get_loc_scales(enlight_ssd_detector_t* param, int idx)
{
    if (idx >= param->loc_tensors_num) {
        enlight_network_err("Wrong loc idx %d\n", idx);
        return 0;
    }

    enlight_act_tensor_t* out_tensor = param->loc_tensors[idx];
    return out_tensor->scale;
}

uint32_t* enlight_ssd_get_conf_logistic_table(enlight_ssd_detector_t* param, int output_index)
{
    return &param->conf_logistic_tables[output_index * 256];
}

enlight_objs_t*   enlight_ssd_get_objs_buf_base(enlight_ssd_detector_t* param)
{
    return &param->detected_objects;
}

uint32_t* enlight_ssd_get_loc_exp_table(enlight_ssd_detector_t* param, int output_index)
{
    return &param->loc_exp_tables[output_index * 256];
}

int enlight_ssd_get_loc_tensors(enlight_ssd_detector_t* param, enlight_act_tensor_t ***tensor_buf)
{
    *tensor_buf = param->loc_tensors;
    return param->loc_tensors_num;
}

int enlight_ssd_get_conf_tensors(enlight_ssd_detector_t* param, enlight_act_tensor_t ***tensor_buf)
{
    *tensor_buf = param->conf_tensors;
    return param->conf_tensors_num;
}

int enlight_ssd_get_bg_in_cls(enlight_ssd_detector_t* param)
{
    return param->bg_in_cls;
}

enlight_logistic_t enlight_ssd_get_logistic(enlight_ssd_detector_t* param)
{
    return param->logistic;
}

enlight_nms_t enlight_ssd_get_nms_method(enlight_ssd_detector_t* param)
{
    return param->nms_method;
}

/******************************************************************************/
/*  YOLO post process extension APIs                                           */
/******************************************************************************/
int enlight_yolo_get_softmax_use(enlight_yolo_detector_t* param)
{
    return param->softmax_use;
}

int enlight_yolo_get_loc_tensors(enlight_yolo_detector_t* param, enlight_act_tensor_t ***tensor_buf)
{
    *tensor_buf = param->loc_tensors;
    return param->loc_tensors_num;
}

int enlight_yolo_get_conf_tensors(enlight_yolo_detector_t* param, enlight_act_tensor_t ***tensor_buf)
{
    *tensor_buf = param->conf_tensors;
    return param->conf_tensors_num;
}

float enlight_yolo_get_thres_conf(enlight_yolo_detector_t* param) 
{
    return param->th_conf;
}

float enlight_yolo_get_thres_iou(enlight_yolo_detector_t* param) 
{
    return param->th_iou;
}

int enlight_yolo_get_num_class(enlight_yolo_detector_t* param) 
{
    return param->num_class;
}

int* enlight_yolo_get_num_grids(enlight_yolo_detector_t* param) 
{
    return param->num_grids;
}

int enlight_yolo_get_num_anchor(enlight_yolo_detector_t* param) 
{
    return param->num_anchor;
}

float enlight_yolo_get_default_box_scale(enlight_yolo_detector_t* param)
{
    return param->default_box_scale;
}

uint8_t* enlight_yolo_get_default_box_buf_addr(enlight_yolo_detector_t* param)
{
    return param->default_box;
}

void enlight_yolo_get_float_default_boxes(enlight_yolo_detector_t* param, float* buf)
{
    int i;
    float scale = param->default_box_scale;
    uint8_t* default_box = param->default_box;
    int num_box;

    num_box = 0;
    for (i=0; i < param->loc_tensors_num; i++) {
        num_box += param->num_grids[i * 2] * param->num_grids[(i * 2) + 1];
    }

    for (i=0; i < num_box; i++)
    {
        buf[i*4] = ((float)default_box[i*4]) / scale;
    }
}

float enlight_yolo_get_output_scales(enlight_yolo_detector_t* param, int idx)
{
    enlight_act_tensor_t* out_tensor = param->output_tensors[idx];
    return (float)out_tensor->scale;
}

uint32_t* enlight_yolo_get_output_exp_table(enlight_yolo_detector_t* param, int output_index)
{
    return &param->exp_tables[output_index * 256];
}

uint32_t* enlight_yolo_get_output_sig_table(enlight_yolo_detector_t* param, int output_index)
{
    return &param->sig_tables[output_index * 256];
}

enlight_objs_t*   enlight_yolo_get_objs_buf_base(enlight_yolo_detector_t* param)
{
    return &param->detected_objects;
}

int enlight_yolo_get_out_tensor_divided(enlight_yolo_detector_t* param)
{
    return param->out_tensor_divided;
}

int enlight_yolo_get_sigmoid_applied(enlight_yolo_detector_t* param)
{
    return param->sigmoid_applied;
}

int enlight_class_get_output_tensors(enlight_classification_t* param, enlight_act_tensor_t ***tensor_buf)
{
    *tensor_buf = param->output_tensors;
    return param->output_tensors_num;
}

int enlight_class_get_num_class(enlight_classification_t* param)
{
    return param->num_class;
}

