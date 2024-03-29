#pragma once

#include <memory>
#include "yaml-cpp/yaml.h"
#include "pipeline_config.h"

namespace hefei {

class PipelineVerifier {
public:    
    PipelineVerifier(int argc, char *argv[]);
    virtual ~PipelineVerifier();

    int init(const std::string& log_level);

    AppConfig &get_app_config() { return m_app_config; }

    int read_config_file(const std::string config_file);

    int read_all_config_files(const char *szFolder);

    void list_pipelines(const std::string &pipeline_name);

    void fork_web_server(int http_port, bool forced);

    void join_web_server();

private:
    int start_web_server(const char *doc_root, int port);

    int read_pipelines_config(YAML::Node &config);
    int read_general_config(YAML::Node &config);
    int read_probe_config(YAML::Node &config);

    std::string m_config_file;
    AppConfig m_app_config;
    std::unique_ptr<std::thread> m_thptr;
};

}// namespace hefei