//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_PIPELINE_CONFIGURATION_YOLOV5_H
#define SOPHON_PIPELINE_CONFIGURATION_YOLOV5_H

#include <fstream>
#include <unordered_map>
#include "json/json.h"

struct CardConfig {
    int devid;
    std::vector<std::string> urls;
    std::vector<std::string> models;
};

struct SConcurrencyConfig {
    int  thread_num {4};
    int  queue_size {4};
    bool blocking   {false};

    SConcurrencyConfig() = default;

    SConcurrencyConfig(Json::Value& value) {
        load(value);
    }

    void load(Json::Value& value) {
        thread_num = value["thread_num"].asInt();
        queue_size = value["queue_size"].asInt();
        blocking   = value["blocking"].asBool();
    }
};

struct SModelConfig {
    std::string name;
    std::string path;
    std::string output_path;
    int   skip_frame;
    int    class_num;
    float class_threshold;
    float obj_threshold;
    float nms_threshold;

    SModelConfig() = default;

    SModelConfig(Json::Value& value) {
        load(value);
    }

    void load(Json::Value& value) {
        name            = value["name"].asString();
        path            = value["path"].asString();
        skip_frame      = value["skip_frame_num"].asInt();
        output_path     = value["output_path"].asString();

        class_threshold = value["class_threshold"].asFloat();
        obj_threshold   = value["obj_threshold"].asFloat();
        nms_threshold   = value["nms_threshold"].asFloat();
        class_num      = value["class_num"].asInt();
    }
};

class Config {
    std::string output_path;
    std::vector<CardConfig> m_cards;
    std::unordered_map<std::string, SModelConfig>       m_models;
    std::unordered_map<std::string, SConcurrencyConfig> m_concurrency;

    void load_cameras(const char* config_file = "cameras.json") {

        Json::Reader reader;
        Json::Value json_root;

        std::ifstream in(config_file);
        if (!in.is_open()) {
            printf("Can't open file: %s\n", config_file);
            return;
        }

        if (!reader.parse(in, json_root, false)) {
            return;
        }


        if (json_root["cards"].isNull() || !json_root["cards"].isArray()){
            in.close();
            return;
        }

        int card_num = json_root["cards"].size();
        for(int card_index = 0; card_index < card_num; ++card_index) {
            Json::Value jsonCard = json_root["cards"][card_index];
            CardConfig card_config;
            card_config.devid = jsonCard["devid"].asInt();
            int camera_num = jsonCard["cameras"].size();
            Json::Value jsonCameras = jsonCard["cameras"];
            for(int i = 0;i < camera_num; ++i) {
                auto json_url_info = jsonCameras[i];
                std::vector<std::string> candidate_models;
                if (json_url_info.isMember("model_names")) {
                    for (Json::ValueIterator itr = json_url_info["model_names"].begin(); itr != json_url_info["model_names"].end(); itr++) {
                        candidate_models.push_back(itr->asString());
                    }
                }
                int chan_num = json_url_info["chan_num"].asInt();
                int loop = candidate_models.size() > 0 ? candidate_models.size() : 1;
                for (int l = 0; l < loop; ++l) {
                    for(int j = 0; j < chan_num; ++j) {
                        auto url = json_url_info["address"].asString();
                        card_config.urls.push_back(url);
                        if (candidate_models.size() > 0) {
                            card_config.models.push_back(candidate_models[l]);
                        }
                    }
                }
                
            }

            m_cards.push_back(card_config);
        }
        // load thread_num, queue_size for concurrency
        if (json_root.isMember("pipeline")) {
            Json::Value pipeline_config = json_root["pipeline"];
            maybe_load_concurrency_cfg(pipeline_config, "preprocess");
            maybe_load_concurrency_cfg(pipeline_config, "inference");
            maybe_load_concurrency_cfg(pipeline_config, "postprocess");
        }
        // load model info
        if (json_root.isMember("models")) {
            int model_num = json_root["models"].size();
            for(int i = 0;i < model_num; ++i) {
                auto model = json_root["models"][i];
                std::string model_name = model["name"].asString();
                if (m_models.count(model_name) != 0) {
                    std::cerr << "ERROR!!! duplicated model config, name: " << model_name << std::endl;
                    continue;
                }
                SModelConfig cfg(model);
                m_models.insert(std::make_pair(model_name, cfg));
            }
        }
        if (json_root.isMember("output_path")) {
            output_path = json_root["output_path"].asString();
        }
        in.close();

    }

public:
    Config(const char* config_file = "cameras.json") {
        load_cameras(config_file);
    }

    int cardNums() {
        return m_cards.size();
    }

    int cardDevId(int index){
        return m_cards[index].devid;
    }

    int totalChanNums() {
        int total_num = 0;
        for (int i = 0; i < m_cards.size(); i++){
            total_num += m_cards[i].urls.size();
        }
        return total_num;
    }

    std::string &get_output_path(){
        return output_path;
    }

    const std::vector<std::string>& cardUrls(int index) {
        return m_cards[index].urls;
    }

    const std::vector<std::string>& cardModels(int index) {
        return m_cards[index].models;
    }

    bool valid_check() {
        if (m_cards.size() == 0) return false;

        for(int i = 0;i < m_cards.size(); ++i) {
            if (m_cards.size() == 0) return false;
        }

        return true;
    }

    bool maybe_load_concurrency_cfg(Json::Value& json_node, const char* phrase) {
        if (json_node.isMember(phrase)) {
            SConcurrencyConfig cfg(json_node[phrase]);
            m_concurrency.insert(std::make_pair(phrase, cfg));
        }
    }

    bool get_phrase_config(const char* phrase, SConcurrencyConfig& cfg) {
        if (m_concurrency.find(phrase) != m_concurrency.end()) {
            cfg = m_concurrency[phrase];
            return true;
        }
        return false;
    }

    const std::unordered_map<std::string, SModelConfig> &getModelConfig() {
        return m_models;
    }

    std::set<std::string> getDistinctModels(int devid) {
        std::set<std::string> st_models(m_cards[devid].models.begin(), m_cards[devid].models.end());
        return std::move(st_models);
    }
};


struct AppStatis {
    int m_channel_num;
    std::mutex m_statis_lock;
    bm::StatToolPtr m_chan_det_fpsPtr;
    bm::StatToolPtr m_total_det_fpsPtr;
    bm::StatToolPtr m_chan_feat_fpsPtr;
    bm::StatToolPtr m_total_feat_fpsPtr;

    uint64_t *m_chan_statis;
    uint64_t m_total_statis = 0;
    uint64_t m_total_decode = 0;
    uint64_t  *m_chan_feat_stat;
    uint64_t  m_total_feat_stat=0;
    uint64_t  m_total_feat_decode=0;


    AppStatis(int num):m_channel_num(num) {
        m_chan_det_fpsPtr = bm::StatTool::create(5);
        m_total_det_fpsPtr = bm::StatTool::create(5);
        m_chan_feat_fpsPtr = bm::StatTool::create(5);
        m_total_feat_fpsPtr = bm::StatTool::create(5);

        m_chan_statis = new uint64_t[m_channel_num];
        memset(m_chan_statis, 0, sizeof(uint64_t)*m_channel_num);
        m_chan_feat_stat = new uint64_t[m_channel_num];
        memset(m_chan_feat_stat, 0, sizeof(uint64_t)*m_channel_num);
        assert(m_chan_feat_stat != nullptr);
    }

    ~AppStatis() {
        delete [] m_chan_statis;
        delete [] m_chan_feat_stat;
    }


};




#endif //SOPHON_PIPELINE_CONFIGURATION_H
