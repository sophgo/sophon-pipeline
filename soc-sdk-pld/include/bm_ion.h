
#ifndef _BM_ION_H_
#define _BM_ION_H_

#include <stdint.h>
#include <stddef.h>

#if defined (__cplusplus)
extern "C" {
#endif

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
#define ATTRIBUTE 
#define DECL_EXPORT __declspec(dllexport)
#define DECL_IMPORT __declspec(dllimport)
#else
#define ATTRIBUTE __attribute__((deprecated))
#define DECL_EXPORT
#define DECL_IMPORT
#endif


/* allocation flags*/
enum {
    BM_ION_FLAG_CACHED = 0,
    BM_ION_FLAG_WRITECOMBINE = 1
};

/*  ION heap type */
enum {
    BM_ION_FLAG_VPP = 0,
    BM_ION_FLAG_NPU = 1,
    BM_ION_FLAG_VPU = 2
};

/* map flags */
enum {
    /* Map memory for CPU write access */
    BM_ION_MAPPING_FLAG_WRITE   = (1 << 0),
    /* Map memory for CPU read access */
    BM_ION_MAPPING_FLAG_READ    = (1 << 1)
};

typedef struct {
    int       memFd;     /* Valid for soc mode */
    uint64_t  paddr;
    uint8_t*  vaddr;
    int       size;

    int       heap_type;
    int       flags;     /* Valid for soc mode */
    int       map_flags; /* Valid for soc mode */

    int       devFd;
    int       soc_idx;   /* Valid for pcie mode */

    /* Don't touch! */
    void*     context;
} bm_ion_buffer_t;

/******************************************************************************
 *
 *    Deprecated Version (Only work in SOC mode)
 *
 ******************************************************************************/

DECL_EXPORT int open_ion_allocator(void) ATTRIBUTE;
DECL_EXPORT int close_ion_allocator(void)  ATTRIBUTE;

DECL_EXPORT bm_ion_buffer_t* ion_allocate_buffer(size_t size, int flags) ATTRIBUTE;
DECL_EXPORT int ion_free_buffer(bm_ion_buffer_t* p) ATTRIBUTE;

DECL_EXPORT int ion_map_buffer(bm_ion_buffer_t* p, int flags) ATTRIBUTE;
DECL_EXPORT int ion_unmap_buffer(bm_ion_buffer_t* p) ATTRIBUTE;

DECL_EXPORT int ion_flush_buffer(bm_ion_buffer_t* p) ATTRIBUTE;
DECL_EXPORT int ion_invalidate_buffer(bm_ion_buffer_t* p) ATTRIBUTE;

/******************************************************************************
 *
 *    Recommended Version
 *
 ******************************************************************************/

DECL_EXPORT int bm_ion_allocator_open(int soc_idx);
DECL_EXPORT int bm_ion_allocator_close(int soc_idx);

DECL_EXPORT bm_ion_buffer_t* bm_ion_allocate_buffer(int soc_idx, size_t size, int flags);
DECL_EXPORT int bm_ion_free_buffer(bm_ion_buffer_t* p);

DECL_EXPORT int bm_ion_upload_data(uint8_t *host_va, bm_ion_buffer_t* p, size_t size);
DECL_EXPORT int bm_ion_download_data(uint8_t *host_va, bm_ion_buffer_t* p, size_t size);

DECL_EXPORT int bm_ion_upload_data2(int soc_idx, uint64_t dst_addr, uint8_t *src_data, size_t size);
DECL_EXPORT int bm_ion_download_data2(int soc_idx, uint64_t src_addr, uint8_t *dst_data, size_t size);

/* Only work in SOC mode */
DECL_EXPORT int bm_ion_map_buffer(bm_ion_buffer_t* p, int flags);
DECL_EXPORT int bm_ion_unmap_buffer(bm_ion_buffer_t* p);

DECL_EXPORT int bm_ion_flush_buffer(bm_ion_buffer_t* p);
DECL_EXPORT int bm_ion_invalidate_buffer(bm_ion_buffer_t* p);


#if defined (__cplusplus)
}
#endif


#endif /* _BM_ION_H_ */

