//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef _SOPHON_PIPELINE_BNUTILITY_BASEMODEL_H_
#define _SOPHON_PIPELINE_BNUTILITY_BASEMODEL_H_
#include <memory>
#include <vector>

#define DEFINE_BASEMODEL_THRES_SETTER_GETTER(T, t)    \
inline void set_##t(T val) { m_##t##_thres = val;  }  \
inline T    get_##t(T val) { return m_##t##_thres; }


namespace bm {

class BaseModel : public std::enable_shared_from_this<BaseModel> {
public:
    DEFINE_BASEMODEL_THRES_SETTER_GETTER(float, cls)
    DEFINE_BASEMODEL_THRES_SETTER_GETTER(float, nms)
    DEFINE_BASEMODEL_THRES_SETTER_GETTER(float, obj)
    inline int getBatch() { return m_max_batch; }
protected:
    BaseModel()           = default;
    virtual ~BaseModel()  = default;

protected:
    float m_cls_thres {0.5};
    float m_nms_thres {0.5};
    float m_obj_thres {0.5};
    int   m_max_batch {1};
    std::vector<std::string> m_class_names;

};
} // namespace bm
#endif // _SOPHON_PIPELINE_BNUTILITY_BASEMODEL_H_