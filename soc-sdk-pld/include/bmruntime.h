/*****************************************************************************
 *
 *    Copyright (c) 2016-2026 by Sophgo Technologies Inc. All rights reserved.
 *
 *    define class bmruntime with all runtime functions used locally, and will
 *    not export to users
 *
 *****************************************************************************/

#ifndef BMRUNTIME_H_
#define BMRUNTIME_H_
#include <mutex>
#include "bmfunc/bmfunc.h"
//#include "bmcpu.h"
#include "bmruntime_common.h"
#include "bmruntime_profile.h"

#include "bmodel.hpp"
#include "bmlib_runtime.h"

using bmodel::CoeffMem;
using bmodel::ModelCtx;
using bmodel::NetParameter;
using flatbuffers::Offset;
using flatbuffers::Vector;

#ifdef _WIN32
#define DECL_EXPORT _declspec(dllexport)
#define DECL_IMPORT _declspec(dllimport)
#else
#define DECL_EXPORT
#define DECL_IMPORT
#endif

#ifndef __linux__
int bmrt_clock_gettime(int dummy, struct timespec* ct);
#endif

namespace bmruntime {

// class defined in this file.
class Bmruntime;
class BmCoeff;
class KernelModule;

struct BmMemory {
  string desc;  // description
  bm_device_mem_t device_mem;
  u8 check_code[bmodel::SHA256_LEN];  // sha256
  u64 addr;
  u32 bytes;
  u32 dword_len;
  bm_handle_t bm_handle;

  void Init(const string& desc, bm_handle_t handle, const bm_device_mem_t& mem, void* buffer);
  int Check();
};

typedef struct subnet_tpu_info {
  subnet_tpu_info()
  {
    gdma_offset = -1;
    bdc_offset = -1;
    cmdgroup_num = -1;
    gdma_group_id_v.clear();
    bdc_group_id_v.clear();
    bdc_cmd_byte_v.clear();
    gdma_cmd_byte_v.clear();
  }

  int is_dynamic;

  vector<u32> gdma_group_id_v;
  vector<u32> bdc_group_id_v;
  vector<u32> gdma_cmd_byte_v;
  vector<u32> bdc_cmd_byte_v;
  int cmdgroup_num;

  u64 gdma_offset;
  u64 bdc_offset;

  u32 ir_offset;
  u32 ir_len;
} SUBNET_TPU_INFO_T;

/* TODO: reuse cpu_layer_param_t */
typedef struct subnet_cpu_info {
  subnet_cpu_info()
  {
    op_type = -1;
    user_param = NULL;
    param_size = 0;
  }
  int op_type;
  void* user_param;
  int param_size;
} SUBNET_CPU_INFO_T;

typedef struct {
    /* for merge subnet, output[i] is selected from {input[output_from[i][0]...input[output_from[i][N]]} */
    vector<vector<int>> output_from;
} SUBNET_MERGE_INFO_T;

typedef struct {
    /* for switch subnet, output[i] is from input[output_from[i]] */
    vector<int> output_from;
    /* for switch subnet, for i-th output, output_branch[i]=0 means the output is to false branch, otherwise to true branch */
    vector<int> output_branch;
    bool valid;
} SUBNET_SWITCH_INFO_T;

#define SUBNET_MODE_TPU 0
#define SUBNET_MODE_CPU 1
#define SUBNET_MODE_MERGE 2
#define SUBNET_MODE_SWITCH 3
typedef struct {
  int subnet_mode; /* 0 : tpu, 1: cpu */
  /* union could not used if include extensible vector */
  SUBNET_TPU_INFO_T tpu_info;
  SUBNET_CPU_INFO_T cpu_info;
  SUBNET_MERGE_INFO_T merge_info;
  SUBNET_SWITCH_INFO_T switch_info;

  /* per subnet i/o tensor */
  vector<string> input_tensor_name_v;
  vector<string> output_tensor_name_v;

  int id;
  vector<int> next_subnet_ids;
} SUBNET_INFO_T;

typedef struct {
  bm_shape_t shape;
  bm_store_mode_t st_mode;
  bm_device_mem_t dev_mem;
  u32 pad_h;
} tensor_attr_t;

typedef enum {
  HOST_MEM_INVALID       = 0,
  HOST_MEM_ALLOC         = 1,   /* Allocated internal, need free */
  HOST_MEM_MMAP          = 2,   /* Mmap from tensor dev_mem,  need unmmap */
  //HOST_MEM_OUTSIDE     = 3,   /* Memory from outside, do nothing ? */
} host_mem_type_t;

typedef struct {
  void*               addr;
  u64                 size;
  host_mem_type_t     type;
} host_mem_t;

typedef enum {
  MEM_TYPE_INVALID  = 0,
  MEM_TYPE_TPU      = (1 << 0),
  MEM_TYPE_CPU      = (1 << 1),
} mem_type_t;

typedef enum {
  TENSOR_TYPE_INVALID        = 0,
  TENSOR_TYPE_NET_INPUT      = (1 << 0),
  TENSOR_TYPE_NET_OUTPUT     = (1 << 1),
  TENSOR_TYPE_IMM_IO         = (1 << 2),
} tensor_io_type_t;

/* record host mem in addition to device mem for
 * cpu layer tensor.
 */
typedef struct {
  bm_tensor_t         tensor_info;
  bm_shape_t          max_shape;
  host_mem_t          host_mem;
  int                 mem_type;
  tensor_io_type_t    io_type;
  int                 io_index;   /* index fro net input/output */
  SUBNET_INFO_T*      src_subnet; /* src subnet for imm i/o tensor */
  int                 record_elem_num; /*if 0, do not use it, the real elem num can be compute from shape. if not 0, use it*/
  unsigned int        pad_h; /* pad_h for conv 3ic */
} tensor_ext_t;

struct net_stage_t {
  vector<tensor_attr_t> input_v;
  vector<tensor_attr_t> output_v;
  u64 coeff_offset;
  u64 ctx_start;
  vector<u64> ctx_offset;
  vector<u64> ctx_borders;

  vector<int> gdma_id;  // for static
  vector<int> bdc_id;   // for static
  vector<u32> gdma_cmd_byte; // for static
  vector<u32> bdc_cmd_byte; // for static
  BmMemory gdma_mem;    // for static
  BmMemory bdc_mem;     // for static
  BmMemory ir_mem;      // for dynamic

  // have multi subnet
  int subnet_num;                  /* subnet num per net */
  vector<SUBNET_INFO_T*> subnet_v; /* subnet per net */

  /* subnet i/o tensor in addtion to net i/o tensor */
  map<string, tensor_ext_t> subnet_tensor_v;
  // save the profile info
  vector<u8> net_profile;
  // save the net stat info
  vector<u8> net_stat;
  // for cpu layer
  u32 cpu_mem_size;
  float* cpu_addr;
};

struct net_ctx_t {
  string net_name;
  vector<string> input_name_v;
  vector<bm_data_type_t> input_type_v;
  vector<float> input_scale_v;
  vector<int> input_zero_point_v;
  vector<string> output_name_v;
  vector<bm_data_type_t> output_type_v;
  vector<float> output_scale_v;
  vector<int> output_zero_point_v;
  vector<net_stage_t *> stage_v;              // each net has multi stages

  // Bulk neuron memories.
  std::vector<bm_device_mem_t> neuron_mem;

  std::mutex neuron_mutex;                    // to avoid neuron mem used by other thread
  bool is_dynamic;
  int n_can_change;                           // for dynamic
  int h_w_can_change;                         // for dynamic
  vector<bm_device_mem_t> middlebuff_input;   // for dynamic, one net share one middlebuf
  vector<bm_device_mem_t> middlebuff_output;  // for dynamic
  bm_net_info_t net_info;                     // create for users by c interface
  std::shared_ptr<KernelModule> kernel_module_;
};

class Bmruntime {
 public:
  Bmruntime(bm_handle_t* bm_handle, bool user_initlized, const string& arch_name);
  Bmruntime(const string& arch_name, int devid);
  ~Bmruntime();

  friend class BMProfile;

  void set_debug_mode(int mode);
  void set_bmrt_mmap(bool enable);
  void subnet_time_print(bool enable);
  bool load_context(const string& ctx_dir);
  bool load_bmodel(const string& filepath);
  bool load_bmodel(const void* bmodel_data, size_t size);

  /* C++ style Interface */
  const vector<string>* get_input_tensor(int net_idx) const;
  const vector<string>* get_output_tensor(int net_idx) const;
  const vector<u8>* get_net_profile(int net_idx, int stage_idx);
  void init_output_tensors(net_ctx_t* net_ctx, net_stage_t* stage,
                                  bm_tensor_t* output_tensors, bool user_mem, bool user_stmode);


  /* C style Interface */
  bool get_input_tensor(int net_idx, int* input_num, const char*** input_names) const;
  bool get_output_tensor(int net_idx, int* output_num, const char*** output_names) const;

  /* use full shape info */
  bool launch(int net_idx, const int input_num, const bm_device_mem_t* input_mems,
              int* input_shapes, int* input_dims, int* in_stmode, int output_num,
              const bm_device_mem_t* output_mems, int* out_stmode, bm_shape_t * output_shapes = NULL);
  bool launch(int net_idx, const bm_tensor_t* input_tensors, int input_num,
              bm_tensor_t* output_tensors, int output_num, bool user_mem = false,
              bool user_stmode = false);
  bool launch(int net_idx, void* const input_datas[], const bm_shape_t input_shapes[],
              int input_num, void* output_tensors[], bm_shape_t output_shapes[], int output_num,
              bool user_mem = false);

  const bm_shape_t* get_input_max_shape(int net_idx, int input_idx);
  const bm_shape_t* get_output_max_shape(int net_idx, int output_idx);
  int get_input_blob_max_shape(const string& tensor_name, int net_idx, int* shape);
  int get_output_blob_max_shape(const string& tensor_name, int net_idx, int* shape);

  // get input and output index by name
  int get_input_index(const string& tensor_name, int net_idx);
  int get_output_index(const string& tensor_name, int net_idx);

  // data_type 0: FP32, 1: FP16, 2: INT8, 3: UINT8, 4: INT16, 5: UINT16
  int get_input_data_type(const string& tensor_name, int net_idx);
  int get_output_data_type(const string& tensor_name, int net_idx);

  // store mode 0: 1N, 1: 2N, 2: 4N
  int get_input_gmem_stmode(const string& tensor_name, int net_idx);
  int get_output_gmem_stmode(const string& tensor_name, int net_idx);

  /* COMMON */
  bool can_batch_size_change(int net_idx);
  bool can_height_and_width_change(int net_idx);

  /* simple get/show */
  void get_network_names(vector<const char*>* names);

  void show_neuron_network();
  inline int get_network_number()
  {
    return m_net_ctx_v.size();
  }

  inline bm_handle_t get_bm_handle()
  {
    return m_handle;
  }
  inline int get_devid(){
    return m_devid;
  }
  int get_net_idx(const string& net_name);
  const bm_net_info_t* get_net_info(int net_idx);

  const std::vector<bm_device_mem_t> &get_neuron_mem(int net_idx);
  void trace();

  size_t size_4N_align(const bm_shape_t& shape, const bm_data_type_t& type);

  u64 must_alloc_device_mem(bm_device_mem_t* mem, u64 size, const string& desc = "", int type_len=1);
  bm_device_mem_t must_alloc_device_mem(u64 size, const string& desc = "", int type_len=1);
  void must_free_device_mem(bm_device_mem_t& mem);

 protected:
  void init();
  void init_bmfunc(const string& arch_name);
  bool launch_static(net_ctx_t* net_ctx, net_stage_t* stage, const bm_tensor_t* input_tensors,
                     int input_num, bm_tensor_t* output_tensors, int output_num);
  bool launch_ir(net_ctx_t* net_ctx, net_stage_t* stage, const bm_tensor_t* input_tensors,
                 int input_num, bm_tensor_t* output_tensors, int output_num);

  int get_stage_idx(const net_ctx_t* net_ctx, const bm_tensor_t* input_tensors);
  int get_static_stage_idx(const net_ctx_t* net_ctx, const bm_tensor_t* input_tensors);
  int get_dynamic_stage_idx(const net_ctx_t* net_ctx, const bm_tensor_t* input_tensors);

 protected:
  // functions for load bmodel
  u64 fix_gdma_addr(const net_stage_t* stage, u64 origin_addr, bool is_src);
  void convert_cmd(u32* cmd, int engine_id, bool last_cmd, u64 start_address,
                   const net_stage_t* stage);
  bool setup_cmd_context(ModelCtx* model_ctx, const Vector<Offset<bmodel::CmdGroup>>* cmd_group,
                         net_stage_t* stage);
  bool setup_ir_context(ModelCtx* model_ctx, const bmodel::Binary* binary_ir,
                        const Vector<Offset<bmodel::StageIR>>* stage_ir, net_stage_t* stage);
  bool load_bmodel(ModelCtx*);
  bool load_bmodel_net(ModelCtx*, int net_idx, std::shared_ptr<KernelModule> kernel_module);
  bool load_bmodel_net(ModelCtx*, int net_idx, net_ctx_t* net_ctx);
  void load_tpu_module(ModelCtx*, std::shared_ptr<KernelModule>& kernel_module);
  bool fill_net_ctx(
      ModelCtx* model_ctx,
      net_ctx_t* net_ctx, const Vector<Offset<NetParameter>>* params,
      std::vector<std::vector<u64>> &stage_ctx_sizes, net_stage_t *stages);
  void fill_net_info(net_ctx_t* net_ctx);
  void free_net_info(net_ctx_t* net_ctx);
  void update_net_middlebuf(net_ctx_t* net_ctx);
  void update_max_middlebuf_size(net_ctx_t* net_ctx);
  void update_max_neuron_mem(const std::vector<u64> &sizes);
  bool setup_profile_context(ModelCtx* model_ctx, net_stage_t* net_stage,
                             const bmodel::Binary* net_profile,
                             const bmodel::Binary* net_stat);

  void set_profile_enabled(bool enable);

 protected:
  static const int MAX_NET_NUM = 256;  // one bmruntime can load 256 nets at most
  vector<net_ctx_t*> m_net_ctx_v;

  bm_handle_t m_handle;
  bool using_internal_bm_handle; /* internal initlized bm_handle or accept from user parameter */
  int m_devid;

  vector<bm_device_mem_t> m_device_mem_vec;     /* save device memory address, for free */

  std::shared_ptr<BmCoeff> m_local_coeff;
  bool m_share_coeff;
  static map<int, std::shared_ptr<BmCoeff>> m_global_coeff_map;
  static std::mutex m_global_coeff_mutex;

  static map<vector<u8>, std::unique_ptr<uint8_t[]>> m_global_cpu_const_map;
  static std::mutex m_global_cpu_const_mutex;

  std::mutex m_load_mutex;

  bool b_enable_mmap;
  bool m_subnet_time_print;

  std::shared_ptr<BMProfile> m_profile;

  // For middle buffer
  // Because max_middle_buffer is also record in m_device_mem_vec.
  // So we do not need to free max_middle_buffer at last.
  bm_device_mem_t max_middle_buffer;
  u64 max_middle_buffer_size;
  u32 middle_buffer_num;

  // For neuron memory share
  u32 m_neuron_heap_mask;
  std::vector<bm_device_mem_t> max_neuron_mem;

 protected:
  /* functions for subnet */
  void bmcpu_setup();
  void bmtpu_setup();
  bool launch_cpu_subnet(net_ctx_t* net_ctx, net_stage_t* stage, SUBNET_INFO_T* subnet,
                         const bm_tensor_t* input_tensors, bm_shape_t real_out_shape[]);
  bool launch_tpu_subnet(net_ctx_t* net_ctx, net_stage_t* stage, SUBNET_INFO_T* subnet,
                         const bm_tensor_t* input_tensors, int input_num,
                         bm_tensor_t* output_tensors, int output_num);
  bool launch_tpu_ir_subnet(net_ctx_t* net_ctx, net_stage_t* stage, SUBNET_INFO_T* subnet,
                            const bm_tensor_t* input_tensors, const int* input_elem_num, int input_num,
                            bm_tensor_t* output_tensors, int* output_elem_num, int output_num);
  bool launch_multi_subnet(net_ctx_t* net_ctx, net_stage_t* stage, const bm_tensor_t* input_tensors,
                           int input_num, bm_tensor_t* output_tensors, int output_num);
  void fill_sub_net(ModelCtx* model_ctx, const Vector<Offset<bmodel::SubNet>>* subnet_set_v,
                    net_ctx_t* net_ctx, net_stage_t* net_stage);
  void fill_subnet_tensor_map(net_ctx_t* net_ctx, net_stage_t* net_stage, SUBNET_INFO_T* subnet,
                              const Vector<Offset<bmodel::Tensor>>* tensor_set_v, bool is_input,
                              std::set<string> subnet_switch_inputs);
  void subnet_clear(net_ctx_t* net_ctx);
  void subnet_tensor_s2d(net_stage_t* net_stage, const string& tensor_name,
                         bm_device_mem_t* out_dev_mem = NULL, u64 offset = 0, u64 size = 0);
  void* subnet_tensor_d2s(net_stage_t* net_stage, const string& tensor_name,
                         bm_device_mem_t* out_dev_mem = NULL, u64 offset = 0, u64 size = 0);
  void subnet_tensor_forward(net_stage_t* stage, const string& src_tensor, const string& dst_tensor, bm_tensor_t* output_tensors);

 protected:
  typedef void* (*t_bmcpu_init)();
  typedef void (*t_bmcpu_uninit)(void*);
  typedef void (*t_bmcpu_process)(void*, int, void*, int, const vector<float*>&,
                                  const vector<vector<int>>&, const vector<float*>&,
                                  vector<vector<int>>&);
  void* bmcpu_handle_;
  t_bmcpu_init bmcpu_init_;
  t_bmcpu_uninit bmcpu_uninit_;
  t_bmcpu_process bmcpu_process_;
  std::shared_ptr<KernelModule> kernel_module_;

 private:
  bmfunc* p_bmfunc;
};

class BmCoeff {
 public:
  explicit BmCoeff(bm_handle_t handle);
  explicit BmCoeff(int devid);
  ~BmCoeff();

  u64 Register(ModelCtx* model_ctx, const CoeffMem* coeff_mem);
  int Check();
  bm_device_mem_t GetCoeffDeviceMem() {
    return m_latest_device_mem;
  }

 protected:
  map<vector<u8>, bm_device_mem_t> m_coeff_map; /* to share the same coeff, by check code*/
  std::mutex m_coeff_mutex;
  bm_handle_t m_handle;
  bool m_inner_handle;
  int m_devid;
  bm_device_mem_t m_latest_device_mem;
};

class KernelModule {
public:
  explicit KernelModule(bm_handle_t &handle, const char* file_name);
  explicit KernelModule(bm_handle_t &handle, const char* binary, size_t size);
  ~KernelModule();
private:
  KernelModule();
private:
  void check_exist() {
    BMRT_ASSERT_INFO(_kernel_module, "kernel_module shouldn't be NULL!!\n");
  }
  void preload_funcs(bm_handle_t &handle);
public:
  tpu_kernel_module_t get_kernel_module();
  tpu_kernel_function_t get_multi_fullnet_func_id();
  tpu_kernel_function_t get_dynamic_fullnet_func_id();
  tpu_kernel_function_t get_enable_profile_func_id();
  tpu_kernel_function_t get_get_profile_func_id();

private:
  bm_handle_t m_handle;
  tpu_kernel_module_t _kernel_module = {0};
  tpu_kernel_function_t _multi_fullnet_func_id = {0};
  tpu_kernel_function_t _dynamic_fullnet_func_id = {0};
  tpu_kernel_function_t _enable_profile_func_id = {0};
  tpu_kernel_function_t _get_profile_func_id = {0};
};

}  // namespace bmruntime

#endif
