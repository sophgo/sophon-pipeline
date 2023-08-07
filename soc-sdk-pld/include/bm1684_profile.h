#ifndef BM1684_PROFILE_H
#define BM1684_PROFILE_H
#include "bmruntime_profile.h"
using namespace bmruntime;

namespace  bm1684_profile {
typedef struct tpu_profile_format {
    unsigned int inst_start_time;
    unsigned int inst_end_time;
    unsigned long long inst_id : 16;
    unsigned long long computation_load : 48;
    unsigned int num_read;
    unsigned int num_read_stall;
    unsigned int num_write;
    unsigned int reserved;
}TPU_PROFILE_FORMAT;

typedef struct gdma_profile_format {
    unsigned int inst_start_time;
    unsigned int inst_end_time;
    unsigned int inst_id : 16;
    unsigned int reserved: 16;
    unsigned int d0_aw_bytes;
    unsigned int d0_wr_bytes;
    unsigned int d0_ar_bytes;
    unsigned int d1_aw_bytes;
    unsigned int d1_wr_bytes;
    unsigned int d1_ar_bytes;
    unsigned int gif_aw_bytes;
    unsigned int gif_wr_bytes;
    unsigned int gif_ar_bytes;
    unsigned int d0_wr_valid_cyc;
    unsigned int d0_rd_valid_cyc;
    unsigned int d1_wr_valid_cyc;
    unsigned int d1_rd_valid_cyc;
    unsigned int gif_wr_valid_cyc;
    unsigned int gif_rd_valid_cyc;
    unsigned int d0_wr_stall_cyc;
    unsigned int d0_rd_stall_cyc;
    unsigned int d1_wr_stall_cyc;
    unsigned int d1_rd_stall_cyc;
    unsigned int gif_wr_stall_cyc;
    unsigned int gif_rd_stall_cyc;
    unsigned int d0_aw_end;
    unsigned int d0_aw_st;
    unsigned int d0_ar_end;
    unsigned int d0_ar_st;
    unsigned int d0_wr_end;
    unsigned int d0_wr_st;
    unsigned int d0_rd_end;
    unsigned int d0_rd_st;
    unsigned int d1_aw_end;
    unsigned int d1_aw_st;
    unsigned int d1_ar_end;
    unsigned int d1_ar_st;
    unsigned int d1_wr_end;
    unsigned int d1_wr_st;
    unsigned int d1_rd_end;
    unsigned int d1_rd_st;
    unsigned int gif_aw_reserved1;
    unsigned int gif_aw_reserved2;
    unsigned int gif_ar_end;
    unsigned int gif_ar_st;
    unsigned int gif_wr_end;
    unsigned int gif_wr_st;
    unsigned int gif_rd_end;
    unsigned int gif_rd_st;
}GDMA_PROFILE_FORMAT;
unsigned int fix_gdma_monitor_data(unsigned char* aligned_data, unsigned int len);

class BMProfileDevice:public bmruntime::BMProfileDeviceBase {

    // BMProfileDeviceBase interface
public:
    BMProfileDevice(BMProfile* profile):BMProfileDeviceBase(profile) {
        enable = profile->getenv_bool(ENV_ENABLE_PROFILE);
        if(enable){
          gdma_record_len = profile->getenv_int(ENV_PROFILE_GDMA_SIZE, gdma_record_len);
          bdc_record_len = profile->getenv_int(ENV_PROFILE_BDC_SIZE, bdc_record_len);
          dyn_max_size = profile->getenv_int(ENV_PROFILE_ARM_SIZE, dyn_max_size);
          enable_gdma = !profile->getenv_bool(ENV_DISABLE_GDMA) && gdma_record_len > 0;
          enable_bdc = !profile->getenv_bool(ENV_DISABLE_BDC) && bdc_record_len > 0;
          enable_arm = !profile->getenv_bool(ENV_DISABLE_ARM) && dyn_max_size > 0;
          enable = enable_gdma || enable_arm || enable_bdc;
        }
    }
    bool init();
    bool begin(net_ctx_t* net_ctx);
    bool end(net_ctx_t* net_ctx);
    void deinit();
    bool enabled();

private:
    buffer_pair tpu_buffer;
    buffer_pair gdma_buffer;
    buffer_pair dyn_buffer;
    bm_perf_monitor tpu_perf_monitor;
    bm_perf_monitor gdma_perf_monitor;
    size_t gdma_record_len = 1024*1024;
    size_t bdc_record_len = 1024*1024;
    size_t dyn_max_size = 16*1024*1024;
    bool enable_gdma =false;
    bool enable_bdc = false;
    bool enable_arm = false;
    bool enable = false;
    bm_device_mem_t aligned_mem;

    // BMProfileDeviceBase interface
};

}
#endif // BM1684_PROFILE_H
