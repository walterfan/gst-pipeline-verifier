#pragma once

#include <memory>
#include "yaml-cpp/yaml.h"
#include "pipeline_config.h"
#include "pipeline_builder.h"

namespace hefei {

class PipelineVerifier {
public:    
    PipelineVerifier(int argc, char *argv[]);
    virtual ~PipelineVerifier();

    int init(const std::string& config_file, const std::string& log_level);

    AppConfig &get_app_config() { return m_app_config; }

    int read_config_file(const std::string config_file);

    int read_all_config_files(const char *szFolder);

    void list_pipelines(const std::string &pipeline_name);

    void fork_web_server(int http_port, bool forced);

    void join_web_server();

    int run_pipeline(const std::string pipeline_name, const std::string variables); 

    int change_pipeline(const PropConfigItem& propConfigItem);

private:
    int start_web_server(const char *doc_root, int port);
    int read_pipeline_config(const std::string &key, const YAML::Node &pipelineNode);
    int read_pipelines_config(YAML::Node &config);
    int read_general_config(YAML::Node &config);
    int read_probe_config(YAML::Node &config);
    int read_props_config(YAML::Node &config);

    std::string m_config_file;
    AppConfig m_app_config;
    std::unique_ptr<std::thread> m_web_thread;

    std::shared_ptr<PipelineBuilder> m_pipeline_builder;
};

}// namespace hefei