#ifndef BMFUNC_H_
#define BMFUNC_H_

#include "bmrt_arch_info.h"
#include "bmfunc/bmdnn_func.h"

namespace bmruntime {

class bmfunc {
  public:
    explicit bmfunc(const string& arch_name);
    virtual ~bmfunc();

    static bmdnn_func*      bmdnn_base() {return sta_bmfunc_ptr->bmdnn_fn;}
    static bmdnn_func_1682* bmdnn_1682() {return sta_bmfunc_ptr->bmdnn_1682_fn;}
    static bmdnn_func_1684* bmdnn_1684() {return sta_bmfunc_ptr->bmdnn_1684_fn;}
    static bmdnn_func_1880* bmdnn_1880() {return sta_bmfunc_ptr->bmdnn_1880_fn;}
    static bmdnn_func_1684x* bmdnn_1684x() {return sta_bmfunc_ptr->bmdnn_1684x_fn;}
    static bmdnn_func_1686* bmdnn_1686() {return sta_bmfunc_ptr->bmdnn_1686_fn;}
    bmrt_arch_info * get_arch_info_ptr() {return p_bmtpu_arch; }
  private:
    static bmfunc*    sta_bmfunc_ptr;  /* why not this ? */
    bmrt_arch_info*   p_bmtpu_arch;

    bmdnn_func*       bmdnn_fn;
    bmdnn_func_1682*  bmdnn_1682_fn;
    bmdnn_func_1684*  bmdnn_1684_fn;
    bmdnn_func_1880*  bmdnn_1880_fn;
    bmdnn_func_1684x* bmdnn_1684x_fn;
    bmdnn_func_1686*  bmdnn_1686_fn;
};

}

#endif
