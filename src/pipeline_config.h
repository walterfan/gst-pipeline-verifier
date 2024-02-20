#pragma once

#include <gst/gst.h>
#include <glib.h>
#include <memory>
#include <thread>
#include <vector>
#include <map>
#include "string_util.h"
#include "file_util.h"

namespace wfan {

struct ElementConfig {
    ElementConfig(const std::string& desc);
    void parse_desc(const std::string& desc);
    void parse_prop(const std::string& token);
    void set_factory(const std::string& factory);
    void insert_prop(const std::string& key, const std::string& value);
    std::string m_name;
    std::string m_factory;
    std::map<std::string, std::string> m_props;
};


struct PipelineConfig {
    PipelineConfig(std::string name, std::vector<std::string> elements);
    
    void insert_desc(const std::string& desc);
    void parse_desc(const std::string& desc);
    
    std::shared_ptr<ElementConfig> get_element_config(const std::string& name);

    std::string m_name;
    std::vector<std::string> m_elements_desc;
    std::vector<std::shared_ptr<ElementConfig>> m_elements_config;
};
}//namespace wfan
