//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//


#ifndef SOPHON_PIPELINE_CONFIGURATION_CVS_H
#define SOPHON_PIPELINE_CONFIGURATION_CVS_H

#include <fstream>
#include <unordered_map>
#include "json/json.h"
#include <set>

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

        class_threshold = value["class_threshold"].asFloat();
        obj_threshold   = value["obj_threshold"].asFloat();
        nms_threshold   = value["nms_threshold"].asFloat();
        class_num      = value["class_num"].asInt();
    }
};

class Config {
    std::vector<CardConfig> m_cards;
    std::unordered_map<std::string, SConcurrencyConfig> m_concurrency;
    std::string path;

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
        path =json_root["models"]["path"].asString();
        int card_num = json_root["cards"].size();
        for(int card_index = 0; card_index < card_num; ++card_index) {
            Json::Value jsonCard = json_root["cards"][card_index];
            CardConfig card_config;

            card_config.devid = jsonCard["devid"].asInt();
            int camera_num = jsonCard["cameras"].size();
            Json::Value jsonCameras = jsonCard["cameras"];
            for(int i = 0;i < camera_num; ++i) {
                auto json_url_info = jsonCameras[i];
                auto url = json_url_info["address"].asString();
                card_config.urls.push_back(url);
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
            return true;
        }
        return false;
    }

    bool get_phrase_config(const char* phrase, SConcurrencyConfig& cfg) {
        if (m_concurrency.find(phrase) != m_concurrency.end()) {
            cfg = m_concurrency[phrase];
            return true;
        }
        return false;
    }

    std::string get_model_path(){
        return path;
    }

    /*const std::unordered_map<std::string, SModelConfig> &getModelConfig() {
        return m_models;
    }*/

    /*std::set<std::string> getDistinctModels(int devid) {
        std::set<std::string> st_models(m_cards[devid].models.begin(), m_cards[devid].models.end());
        return std::move(st_models);
    }*/
};


struct AppStatis {
    int m_channel_num;
    std::mutex m_statis_lock;
    std::mutex m_statis_decode_lock;
    std::mutex m_statis_encode_lock;

    bm::StatToolPtr m_total_decode_fpsPtr;
    bm::StatToolPtr m_total_encode_fpsPtr;

    bm::StatToolPtr m_chan_det_fpsPtr;
    bm::StatToolPtr m_total_det_fpsPtr;
    bm::StatToolPtr m_chan_feat_fpsPtr;
    bm::StatToolPtr m_total_feat_fpsPtr;
    
    uint64_t *m_chan_statis;
    uint64_t m_total_statis = 0;
    uint64_t m_total_decode = 0;
    uint64_t  m_total_encode = 0;

    uint64_t  *m_chan_feat_stat;
    uint64_t  m_total_feat_stat=0;
    uint64_t  m_total_feat_decode=0;


    AppStatis(int num):m_channel_num(num) {
        m_total_decode_fpsPtr = bm::StatTool::create(5);
        m_total_encode_fpsPtr = bm::StatTool::create(5);
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
