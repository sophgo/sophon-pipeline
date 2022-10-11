
#ifndef SOPHON_PIPELINE_BM_TRACKER_H
#define SOPHON_PIPELINE_BM_TRACKER_H

#include "bmutility_types.h"
#include <memory>
namespace bm {
    class BMTracker {
    public:
        virtual ~BMTracker() {}

        static std::shared_ptr<BMTracker> create(float max_cosine_distance=0.2, int nn_budget=100);
        virtual void update(const bm::NetOutputObjects &rects, bm::NetOutputObjects &tracks) = 0;

    };

}

#endif //SOPHON_PIPELINE_BM_TRACKER_H
