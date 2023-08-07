#ifndef BMRUNTIME_PROFILE_H
#define BMRUNTIME_PROFILE_H
#ifdef __linux__
#include <sys/time.h>
#endif
#include <string.h>
#include <memory>
#include <functional>
#include "bmrt_arch_info.h"
#include "bmruntime_common.h"
#ifdef __linux__
#define TIME_TO_USECS(t) (t.tv_sec*1000000+ t.tv_usec)
#else
#define TIME_TO_USECS(t) (t.tv_sec*1000000+ t.tv_nsec)
#endif

#define ENV_ENABLE_PROFILE "BMRUNTIME_ENABLE_PROFILE"
#define ENV_PROFILE_GDMA_SIZE "BMRUNTIME_GDMA_RECORD_SIZE"
#define ENV_PROFILE_BDC_SIZE "BMRUNTIME_BDC_RECORD_SIZE"
#define ENV_PROFILE_ARM_SIZE "BMRUNTIME_ARM_RECORD_SIZE"

#define ENV_DISABLE_GDMA "BMRUNTIME_DISABLE_GDMA_PERF"
#define ENV_DISABLE_BDC "BMRUNTIME_DISABLE_BDC_PERF"
#define ENV_DISABLE_ARM "BMRUNTIME_DISABLE_ARM_PERF"

namespace bmruntime {

typedef enum {
    BLOCK_SUMMARY      = 1,
    BLOCK_COMPILER_LOG = 2,
    BLOCK_MONITOR_BDC  = 3,
    BLOCK_MONITOR_GDMA = 4,
    BLOCK_DYN_DATA     = 5,
    BLOCK_DYN_EXTRA    = 6,
    BLOCK_FIRMWARE_LOG = 7,
    BLOCK_CMD          = 8,
    BLOCK_BMLIB        = 9,
} profile_block_type_t;

typedef enum {
    MEM_CPU = 0,
    MEM_GLOBAL = 1,
    MEM_LOCAL = 2,
    MEM_ARM = 3,
} PROFILE_MEM_TYPE_T;

typedef struct {
    PROFILE_MEM_TYPE_T type;
    u64 addr;
    u32 size;
    u64 alloc_usec;
    u64 free_usec;
    string desc;
} profile_mem_info_t;

#pragma pack(1)
typedef struct {
    int iteration;
    int subnet_id;
    int subnet_type; //TPU, CPU, SWITCH, MERGE
    u64 start_usec;
    u64 end_usec;
    u64 extra_data; //for extend data, TPU: static or dynamic, SWITCH: cond, CPU_LAYER: cpu_op
} profile_subnet_summary_t;
#pragma pack()

#pragma pack(1)
typedef struct {
    u32 gdma;
    u32 bdc;
} profile_cmd_num_t;
typedef struct {
    u64 gdma_base_addr;
    u64 gdma_offset;
    u64 bdc_base_addr;
    u64 bdc_offset;
    u32 group_num;
    profile_cmd_num_t cmd_num[0];
} profile_cmd_info_t;
#pragma pack()

u64 get_usec();

typedef struct {
    size_t size = 0;
    u8* ptr = nullptr;
    bm_device_mem_t mem;
} buffer_pair;

class BMProfile;
class Bmruntime;
struct net_ctx_t;

// struct net_ctx_t;
class BMProfileDeviceBase {
public:
    BMProfileDeviceBase(BMProfile* profile):profile(profile) {}
    virtual bool enabled() = 0;
    virtual bool init() = 0;
    virtual bool begin(net_ctx_t* net_ctx) = 0;
    virtual bool end(net_ctx_t* net_ctx) = 0;
    virtual void deinit() = 0;
    virtual ~BMProfileDeviceBase(){}
protected:
    BMProfile* profile;
};

class BMProfile {
public:
    BMProfile(Bmruntime* p_bmrt);
    ~BMProfile();

    void set_enable(bool is_enable,
                    set<int> iterations=set<int>(),
                    set<int> subnet_ids = set<int>(),
                    set<int> subnet_modes = set<int>());
    void init(const std::string &net_name, const vector<u8> &data, const vector<u8> &stat);
    void begin_subnet(net_ctx_t* net_ctx, int iteration, int subnet_id, int subnet_mode);
    void set_extra_data(u64 data);
    void end_subnet(net_ctx_t* net_ctx);
    void deinit();
    void print_note();
    bool is_enabled() { return enabled; }
    void record_alloc_device_mem(const bm_device_mem_t& mem, const string& desc="");
    void record_cpu_mem(const void* ptr, u32 len, const string& desc="");
    void record_mem(PROFILE_MEM_TYPE_T mtype, u64 addr, u32 size, const string& desc="");
    void record_free_device_mem(const bm_device_mem_t &mem);

    profile_cmd_num_t* record_subnet_cmd_info(u64 gdma_addr, u64 gdma_offset, u64 bdc_addr, u64 bdc_offset, u32 group_num);
    void record_cmd_data(ENGINE_ID engine, const void* cmd_ptr, u32 cmd_len, u64 store_addr);

private:
    profile_subnet_summary_t summary;
    int current_enabled=false;

    void end_profile(net_ctx_t* net_ctx);

    string get_save_dir() const;
    void set_save_dir(const string &value);
    string get_global_filename();


  public:
    bool need_profile(int iteration, int subnet_id, int subnet_mode);
    void alloc_buffer(buffer_pair* bp, size_t size, const std::string &desc);
    void free_buffer(buffer_pair* bp);

    void create_file();
    void write_block(u32 type, size_t len, const void *data);
    void close_file();

    void save_cmd_profile();
    int getenv_int(const char* name, int default_val = 0);
    bool getenv_bool(const char* name, bool default_val = false);

    bm_handle_t handle = nullptr;
private:
    Bmruntime* p_bmrt = nullptr;
    profile_cmd_info_t *cmd_info = nullptr;
    int arch = -1;
    int devid = -1;
    bool enabled = false;

    set<int> enabled_subnet_ids;
    set<int> enabled_iterations;
    set<int> enabled_subnet_modes;

    FILE* profile_fp = nullptr;

    std::shared_ptr<BMProfileDeviceBase> device;

    string save_dir = "bmprofile_data";
    vector<profile_mem_info_t> mem_info;
};
}

#endif // BMRUNTIME_PROFILE_H
