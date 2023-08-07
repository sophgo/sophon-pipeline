#ifndef BMLIB_MIDDLEWARE_H_
#define BMLIB_MIDDLEWARE_H_

#include "bmlib_runtime.h"

#if defined(_WIN32) && !defined(__MINGW32__)
    #define DECL_EXPORT __declspec(dllexport)
    #define DECL_IMPORT __declspec(dllimport)
#else
    #define DECL_EXPORT
    #define DECL_IMPORT
#endif


#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
  DEV_FD = 0,
  ION_FD = 1,
  SPACC_FD = 2,
  VPP_FD = 3,
  END_FD = 4
} FD_ID;

/**
 * @name    bm_get_handle_fd
 * @brief   get fd value from handle
 * @ingroup device management api
 *
 * @param [in]   handle The device handle
 * @param [in]   fd index
 * @param [out]  fd
 * @retval  BM_SUCCESS  Succeeds.
 *          Other code  Fails.
 */
DECL_EXPORT bm_status_t bm_get_handle_fd(bm_handle_t handle,FD_ID id, int *fd);


#if defined(__cplusplus)
}
#endif

#endif /* BMLIB_MIDDLEWARE_H_ */
