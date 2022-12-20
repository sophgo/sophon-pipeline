//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#pragma once
#include<iostream>

#define call(fn, ...) \
    do { \
        auto ret = fn(__VA_ARGS__); \
        if (ret != BM_SUCCESS) \
        { \
            std::cout << "[ERROR] " << #fn << " failed " << ret << std::endl; \
            throw std::runtime_error("api error"); \
        } \
    } while (false);

