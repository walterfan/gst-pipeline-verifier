#pragma once

#include <memory>
#include "pipeline_config.h"

namespace hefei {

class PipelineVerifier {
public:    
    PipelineVerifier(int argc, char *argv[]);
    virtual ~PipelineVerifier();

    int init();

    AppConfig &get_app_config() { return m_app_config; }

    int read_config_file(const char *szFile);

    int read_all_config_files(const char *szFolder);

    void list_pipelines(const std::string &pipeline_name);

private : 
    std::string m_config_file;
    AppConfig m_app_config;
    
};

}// namespace hefei