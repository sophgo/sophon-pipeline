//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_PIPELINE_BMUTILITY_STRING_H
#define SOPHON_PIPELINE_BMUTILITY_STRING_H
#include <string>
#include <vector>
#include "bmutility_types.h"

namespace bm {
    std::vector<std::string> split(std::string str, std::string pattern);
    bool start_with(const std::string &str, const std::string &head);
    std::string file_name_from_path(const std::string& path, bool hasExt);
    std::string file_ext_from_path(const std::string& str);
    std::string format(const char *pszFmt, ...);

    std::string base64_enc(const void *data, size_t sz);
    std::string base64_dec(const void *data, size_t sz);

}
#endif //SOPHON_PIPELINE_BMUTILITY_STRING_H
