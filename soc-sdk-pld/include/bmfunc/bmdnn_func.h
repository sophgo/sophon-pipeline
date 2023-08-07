#ifndef BMDNN_FUNC_H_
#define BMDNN_FUNC_H_

#include "bmruntime_common.h"

namespace bmruntime {

class bmdnn_func {
  public:
    bmdnn_func() {};
    ~bmdnn_func() {};
};

class bmdnn_func_1682 : public bmdnn_func {
  public:

    bmdnn_func_1682() {
        BM_API_ID_MULTI_FULLNET      = 134;
        BM_API_ID_DYNAMIC_FULLNET    = 135;
        BM_API_ID_MULTI_FULLNET_PROFILE = 137;
        MAX_API_MSG_SIZE             = 1019 * sizeof(u32);  //ref to 1682
    };
    void set_bmdnn_func_profile(bool enable) {
        b_enable_profile = enable;
    }
    bm_status_t _bmdnn_multi_fullnet_(
            bm_handle_t handle,
            int input_num,
            unsigned long long* user_input_global_offset,
            unsigned long long* cmd_input_global_offset,
            int* input_tensor_size,
            unsigned short* input_dtype,
            int output_num,
            unsigned long long* user_output_global_offset,
            unsigned long long* cmd_output_global_offset,
            int* output_tensor_size,
            unsigned short* output_dtype,
            unsigned long long bdc_cmd_offset,
            unsigned long long gdma_cmd_offset,
            unsigned long long cdma_cmd_offset,
            int* bdc_cmd_num,
            int* gdma_cmd_num,
            int* cdma_cmd_num,
            int cmdgroup_num
            );
#if 0
    // legacy interface
    bm_status_t _bmdnn_dynamic_fullnet_(
            bm_handle_t handle,
            unsigned long long compiled_ir_global_addr,
            unsigned int compiled_ir_length,  //unit dword
            unsigned int batch_num,
            unsigned int input_num,
            unsigned long long* input_global_offset,
            unsigned int* input_c,
            unsigned int* input_height,
            unsigned int* input_width,
            unsigned int output_num,
            unsigned long long* output_global_offset,
            unsigned long long apd_ctx_mem_offset,
            bool               get_output_shape,
            unsigned long long output_shape_global_addr
            );

    bm_status_t _bmdnn_dynamic_fullnet_ex_(
            bm_handle_t handle,
            unsigned long long compiled_ir_global_addr,
            unsigned int compiled_ir_length,  //unit dword
            unsigned int batch_num,
            unsigned int input_num,
            unsigned long long* input_global_offset,
            unsigned int* input_c,
            unsigned int* input_height,
            unsigned int* input_width,
            unsigned int output_num,
            unsigned long long* output_global_offset,
            unsigned long long apd_ctx_mem_offset,
            unsigned long long apd_coeff_mem_offset,
            bool               get_output_shape,
            unsigned long long output_shape_global_addr
            );
#endif

    bm_status_t _bmdnn_dynamic_fullnet_v2_(
        bm_handle_t handle,
        unsigned long long compiled_ir_global_addr,
        unsigned int compiled_ir_length,  //unit dword
        unsigned int input_num,
        const unsigned long long *input_addrs,
        const int * const * input_shapes,
        const int * input_elem_nums,
        const int * input_dtype_and_dims,
        unsigned int output_num,
        const unsigned long long *output_addrs,
        unsigned long long apd_ctx_start,
        unsigned long long apd_ctx_mem_offset,
        unsigned long long apd_coeff_mem_offset,
        bool get_output_shape,
        unsigned long long output_shape_global_addr,
        unsigned int using_arm_buffer_size
    );
  private:
    bool b_enable_profile = false;
    u32 BM_API_ID_MULTI_FULLNET_PROFILE;
    u32 BM_API_ID_MULTI_FULLNET;
    u32 BM_API_ID_DYNAMIC_FULLNET;
    u32 MAX_API_MSG_SIZE;
};

class bmdnn_func_1684 : public bmdnn_func {
  public:

    bmdnn_func_1684() {
        BM_API_ID_MULTI_FULLNET       = 110;
        BM_API_ID_DYNAMIC_FULLNET     = 111;
        BM_API_ID_SET_PROFILE_ENABLE  = 986;
        BM_API_ID_GET_PROFILE_DATA    = 987;
        MAX_API_MSG_SIZE              = 1022 * sizeof(u32); // ref to 1684
    };
    bm_status_t _bmdnn_multi_fullnet_(
        bm_handle_t handle,
        int input_num,
        u64* user_input_global_offset,
        u64* cmd_input_global_offset,
        int* input_n,
        int* input_c,
        int* input_h,
        int* input_w,
        unsigned short* input_data_type,
        unsigned char* input_st_mode,
        unsigned char* real_in_stmode,
        int output_num,
        u64* user_output_global_offset,
        u64* cmd_output_global_offset,
        int* output_n,
        int* output_length,
        unsigned short* output_data_type,
        unsigned char* output_st_mode,
        unsigned char* force_out_stmode,
        u64 bdc_cmd_offset,
        u64 gdma_cmd_offset,
        int* bdc_cmd_num,
        int* gdma_cmd_num,
        int cmdgroup_num,
        u32* input_pad_h);

    bm_status_t _bmdnn_dynamic_fullnet_v2_(
        bm_handle_t handle,
        unsigned long long compiled_ir_global_addr,
        unsigned int compiled_ir_length,  //unit dword
        unsigned int input_num,
        const unsigned long long *input_addrs,
        const unsigned long long *input_middle_addrs,
        const int * const * input_shapes,
        const int * input_elem_nums,
        const int * input_dtype_and_dims,
        unsigned int output_num,
        const unsigned long long *output_addrs,
        const unsigned long long *output_middle_addrs,
        unsigned long long apd_ctx_start,
        std::vector<unsigned long long> apd_ctx_mem_borders,
        std::vector<unsigned long long> apd_ctx_mem_offset,
        unsigned long long apd_coeff_mem_offset,
        bool need_middle_buff_flag,
        unsigned int* output_need_middle_buff_flag,
        bool get_output_shape,
        unsigned long long output_shape_global_addr,
        unsigned int using_arm_buffer_size
    );
    bm_status_t _bmdnn_set_profile_enable_(bm_handle_t handle, bool enable);
    bm_status_t _bmdnn_get_profile_data_(bm_handle_t handle,
                                         unsigned long long output_global_addr,
                                         unsigned int output_max_size,
                                         unsigned int offset,
                                         unsigned int data_category //0: profile time records, 1: extra data
                                         );

  private:
    u32 BM_API_ID_MULTI_FULLNET;
    u32 BM_API_ID_DYNAMIC_FULLNET;
    u32 BM_API_ID_SET_PROFILE_ENABLE;
    u32 BM_API_ID_GET_PROFILE_DATA;
    u32 MAX_API_MSG_SIZE;
};

class bmdnn_func_1880 : public bmdnn_func {
  public:

    bmdnn_func_1880() {
        ;
    };
    bm_status_t _bmdnn_multi_fullnet_(
        bm_handle_t handle,
        int input_num,
        u64* user_input_global_offset,
        u64* cmd_input_global_offset,
        int* input_n,
        int* input_length,
        unsigned short* input_data_type,
        unsigned char* input_st_mode,
        unsigned char* real_in_stmode,
        int output_num,
        u64* user_output_global_offset,
        u64* cmd_output_global_offset,
        int* output_n,
        int* output_length,
        unsigned short* output_data_type,
        unsigned char* output_st_mode,
        unsigned char* force_out_stmode,
        u64 bdc_cmd_offset,
        u64 gdma_cmd_offset,
        int* bdc_cmd_num,
        int* gdma_cmd_num,
        int cmdgroup_num
        );

    bm_status_t _bmdnn_dynamic_fullnet_v2_(
        bm_handle_t handle,
        unsigned long long compiled_ir_global_addr,
        unsigned int compiled_ir_length,
        unsigned int input_num,
        const unsigned long long *input_addrs,
        const unsigned long long *input_middle_addrs,
        const int * const * input_shapes,
        const int * input_dims,
        unsigned int output_num,
        const unsigned long long *output_addrs,
        const unsigned long long *output_middle_addrs,
        unsigned long long apd_ctx_mem_offset,
        unsigned long long apd_coeff_mem_offset,
        bool need_middle_buff_flag,
        unsigned int* output_need_middle_buff_flag,
        bool get_output_shape,
        unsigned long long output_shape_global_addr,
        unsigned int using_arm_buffer_size
    );

  private:
};

class bmdnn_func_1684x : public bmdnn_func {
  public:

    bmdnn_func_1684x() {
        SG_API_ID_MULTI_FULLNET       = 0x0ffffffb;
        SG_API_ID_DYNAMIC_FULLNET     = 0x0ffffffc;
        SG_API_ID_SET_PROFILE_ENABLE  = 986;
        SG_API_ID_GET_PROFILE_DATA    = 987;
        MAX_API_MSG_SIZE              = 1016 * sizeof(u32);
    };
    bm_status_t _bmdnn_multi_fullnet_(
        bm_handle_t handle,
        int func_id,
        int input_num,
        u64* user_input_global_offset,
        u64* cmd_input_global_offset,
        u32* input_dsize,  // in bytes
        int output_num,
        u64* user_output_global_offset,
        u64* cmd_output_global_offset,
        u32* output_dsize, // in bytes
        u64 bdc_cmd_offset,
        u64 gdma_cmd_offset,
        int* bdc_cmd_num,
        int* gdma_cmd_num,
        u32* bdc_cmd_byte_size,
        u32* gdma_cmd_byte_size,
        int cmdgroup_num);

    bm_status_t _bmdnn_dynamic_fullnet_(
        bm_handle_t handle,
        int func_id,
        unsigned long long compiled_ir_global_addr,
        unsigned int compiled_ir_length, //unit dword
        unsigned int input_num,
        const unsigned long long *input_addrs,
        const int * const * input_shapes,
        const int * input_elem_nums,
        const int * input_dtype_and_dims,
        unsigned int output_num,
        const unsigned long long *output_addrs,
        unsigned long long apd_ctx_start,
        std::vector<unsigned long long> apd_ctx_mem_borders,
        std::vector<unsigned long long> apd_ctx_mem_offset,
        unsigned long long apd_coeff_mem_offset,
        bool get_output_shape,
        unsigned long long output_shape_global_addr);

    bm_status_t _bmdnn_set_profile_enable_(bm_handle_t handle, tpu_kernel_function_t func_id, bool enable);
    bm_status_t _bmdnn_get_profile_data_(bm_handle_t handle,
                                         tpu_kernel_function_t func_id,
                                         unsigned long long output_global_addr,
                                         unsigned int output_max_size,
                                         unsigned int offset,
                                         unsigned int data_category //0: profile time records, 1: extra data
                                         );
  private:
    u32 SG_API_ID_MULTI_FULLNET;
    u32 SG_API_ID_DYNAMIC_FULLNET;
    u32 SG_API_ID_SET_PROFILE_ENABLE;
    u32 SG_API_ID_GET_PROFILE_DATA;
    u32 MAX_API_MSG_SIZE;
};

class bmdnn_func_1686 : public bmdnn_func {
  public:

    bmdnn_func_1686() {
        SG_API_ID_MULTI_FULLNET       = 0x0ffffffb;
        SG_API_ID_DYNAMIC_FULLNET     = 0x0ffffffc;
        SG_API_ID_SET_PROFILE_ENABLE  = 986;
        SG_API_ID_GET_PROFILE_DATA    = 987;
        MAX_API_MSG_SIZE              = 1016 * sizeof(u32);
    };
    bm_status_t _bmdnn_multi_fullnet_(
        bm_handle_t handle,
        int input_num,
        u64* user_input_global_offset,
        u64* cmd_input_global_offset,
        u32* input_dsize,  // in bytes
        int output_num,
        u64* user_output_global_offset,
        u64* cmd_output_global_offset,
        u32* output_dsize, // in bytes
        u64 bdc_cmd_offset,
        u64 gdma_cmd_offset,
        int* bdc_cmd_num,
        int* gdma_cmd_num,
        u32* bdc_cmd_byte_size,
        u32* gdma_cmd_byte_size,
        int cmdgroup_num);

    bm_status_t _bmdnn_dynamic_fullnet_(
        bm_handle_t handle,
        unsigned long long compiled_ir_global_addr,
        unsigned int compiled_ir_length, //unit dword
        unsigned int input_num,
        const unsigned long long *input_addrs,
        const int * const * input_shapes,
        const int * input_elem_nums,
        const int * input_dtype_and_dims,
        unsigned int output_num,
        const unsigned long long *output_addrs,
        unsigned long long apd_ctx_start,
        std::vector<unsigned long long> apd_ctx_mem_borders,
        std::vector<unsigned long long> apd_ctx_mem_offset,
        unsigned long long apd_coeff_mem_offset,
        bool get_output_shape,
        unsigned long long output_shape_global_addr);

    bm_status_t _bmdnn_set_profile_enable_(bm_handle_t handle, bool enable);
    bm_status_t _bmdnn_get_profile_data_(bm_handle_t handle,
                                         unsigned long long output_global_addr,
                                         unsigned int output_max_size,
                                         unsigned int offset,
                                         unsigned int data_category //0: profile time records, 1: extra data
                                         );
  private:
    u32 SG_API_ID_MULTI_FULLNET;
    u32 SG_API_ID_DYNAMIC_FULLNET;
    u32 SG_API_ID_SET_PROFILE_ENABLE;
    u32 SG_API_ID_GET_PROFILE_DATA;
    u32 MAX_API_MSG_SIZE;
};
}

#endif
