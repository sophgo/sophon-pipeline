
#ifndef __BM_VPU_TYPES_H__
#define __BM_VPU_TYPES_H__

#include <stdint.h>
#include <stddef.h>

#ifndef bm_pa_t
# if defined(__aarch64__) || defined(__amd64__) || defined(__x86_64__) || defined(__sw_64__) || defined(__loongarch64) || (defined(__mips__) &&(__mips == 64))
typedef uint64_t bm_pa_t;
# elif _WIN32
typedef unsigned long long  bm_pa_t;
# else
typedef uint32_t bm_pa_t;
# endif
#endif

#ifndef BYTE
typedef uint8_t BYTE;
#endif

#ifndef BOOL
typedef int     BOOL;
#endif

#ifndef TRUE
# define TRUE    1
#endif

#ifndef FALSE
# define FALSE   0
#endif

#ifndef NULL
# define NULL    0
#endif

#ifdef _WIN32
typedef unsigned long long u64;
typedef unsigned int u32;
#elif __linux__
typedef unsigned long u64;
#endif

#ifndef UNREFERENCED_PARAMETER
# define UNREFERENCED_PARAMETER(P)          \
    /*lint -save -e527 -e530 */ \
{ \
    (P) = (P); \
} \
    /*lint -restore */
#endif

#ifdef __GNUC__
# define UNREFERENCED_FUNCTION __attribute__ ((unused))
#else
# define UNREFERENCED_FUNCTION
#endif

#endif  /* __BM_VPU_TYPES_H__ */

