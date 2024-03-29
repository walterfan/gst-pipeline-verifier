#pragma once

#include <gst/gst.h>
#include <glib.h>
#include <memory>
#include <thread>
#include <vector>
#include <map>
#include "string_util.h"
#include "file_util.h"

namespace hefei {


struct ElementConfig {
    ElementConfig(const std::string& desc);
    void parse_desc(const std::string& desc);
    void parse_prop(const std::string& token);
    void set_factory(const std::string& factory);
    void insert_prop(const std::string& key, const std::string& value);

    std::string m_name;
    std::string m_factory;
    std::string m_fork_tag;
    std::string m_link_tag;
    std::map<std::string, std::string> m_props;
};


struct PipelineConfig {
    PipelineConfig(std::string name, std::vector<std::string> elements);
    
    std::shared_ptr<ElementConfig> get_element_config(const std::string& name);

    int check_elements_name();
    
    //pipeline name
    std::string m_name;

    //elements' description
    std::vector<std::string> m_elements_desc;

    //parsed elements configuration
    std::vector<std::shared_ptr<ElementConfig>> m_elements_config;
};

struct GeneralConfig {
    int http_enabled = 0;
    int http_port = 9988;
    int debug_threshold = 3;
    int log_level = 2; 
    std::string log_folder;
    std::string log_name;
    std::string default_pipeline;
    std::string web_root = "./doc";
};


struct ProbeConfigItem {
    std::string probe_pipeline_name;
    std::string probe_element_name;
    std::string probe_pad_name;
    int probe_type;
};

struct ProbeConfig {
    bool has_probe_config_item(const std::string& item_name);
    ProbeConfigItem& get_probe_config_item(const std::string& item_name);
    void add_probe_config_item(const ProbeConfigItem& ProbeConfigItem);
    std::map<std::string, ProbeConfigItem> config_items;

};


class AppConfig {
public:
    GeneralConfig& get_general_config() { return m_general_config; };
    ProbeConfig& get_probe_config()  { return m_probe_config; };
    pipeline_config_t &get_pipeline_config() { return m_pipeline_config; }
    bool has_probe_config_item(const std::string& item_name) { return m_probe_config.has_probe_config_item(item_name); }
    ProbeConfigItem& get_probe_config_item(const std::string& item)  { return m_probe_config.get_probe_config_item(item); };
private:
    GeneralConfig m_general_config;
    ProbeConfig m_probe_config;
    pipeline_config_t m_pipeline_config;
};

}//namespace hefei
