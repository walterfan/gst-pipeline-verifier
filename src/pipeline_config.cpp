#include "gst_util.h"
#include "file_util.h"
#include "string_util.h"
#include "pipeline_config.h"
#include "logger.h"

using namespace std;

static const std::string DOT = ".";

namespace wfan {

ElementConfig::ElementConfig(const std::string& desc) {
    parse_desc(desc);
}

void ElementConfig::set_factory(const std::string& factory) {
    if ((startswith(factory, "\"") && endswith(factory, "\"")) 
        || (startswith(factory, "\'") && endswith(factory, "\'"))) {
        
        insert_prop("caps", factory);
        m_factory = "capsfilter";
        m_name = m_name.empty()? m_factory: m_name; 
        return;
    }
    m_factory = factory;
    trim_space(m_factory);
    if (m_name.empty()) {
        m_name = m_factory;
    }
}


void ElementConfig::insert_prop(const std::string& key, const std::string& value) {
    if (key.empty() || value.empty()) {
        return;
    }
    if (key == "name") {
        m_name = value;
        return;
    }
    m_props[key] = value;
    DLOG("key={}, value={}", key, value);        
}

void ElementConfig::parse_prop(const std::string& token) {
    if (token.empty()) {
        return;
    }
    
    size_t pos = 0;
    if ((pos = token.find("=")) != string::npos) {
        insert_prop(trim_copy(token.substr(0, pos)), trim_copy(token.substr(pos+1)));
    } else {
        if (m_factory.empty() && token.find(DOT) == std::string::npos) {
            set_factory(trim(token, " \t"));
        } else {
            //stop and start another branch
            if (endswith(token, DOT)) {
                m_fork_tag = trim(token, " \t.");
                DLOG("fork tag: {}", m_fork_tag);
            } else if(token.find(DOT) != std::string::npos) {
                m_link_tag = trim(token, " \t");
                DLOG("link tag: {}", m_link_tag);
            } else {
                WLOG("unknown token: {}", token);
            }
        }

    }
}

void ElementConfig::parse_desc(const std::string& desc) {

    std::vector<std::string> tokens;
    int count = tokenize(desc, tokens);

    if (count == 0) {
        return;
    }

    if (count == 1 && m_factory.empty()) {
        set_factory(trim(tokens[0], " \t"));
        DLOG("m_factory: {}", m_factory);
        return;
    }

    for (int i = 0; i < count; ++i) {
        auto& token = tokens[i];
        parse_prop(token);
    }
}


PipelineConfig::PipelineConfig(std::string name, std::vector<std::string> elements)
: m_name(name)
, m_elements_desc(elements) {
    DLOG("create pipeline {}", m_name);
    for(auto& desc: m_elements_desc) {
        m_elements_config.push_back(std::make_shared<ElementConfig>(desc));
    }
}

std::shared_ptr<ElementConfig> PipelineConfig::get_element_config(const std::string& name) {
    for(auto& ele_cfg_ptr: m_elements_config) {
        if (ele_cfg_ptr->m_name == name) {
            return ele_cfg_ptr;
        }
    }
    return nullptr;
}

int PipelineConfig::check_elements_name() {
    size_t count = m_elements_config.size();
    
    size_t i = 0, j = 0;

    for(; i < count - 1; ++i) {
        auto& ele_cfg_ptr = m_elements_config[i];
        auto& element_name = ele_cfg_ptr->m_name;
        size_t index = 0;
        for (j = i + 1; j < count; ++j) {
            auto& next_ptr = m_elements_config[j];
            auto& next_name = next_ptr->m_name;
            if (next_name == element_name) {
                next_name = element_name + std::to_string(++index);
                DLOG("{} rename to {}", element_name, next_name);
            }            
        }
    }
    return 0;
}

} //namespace wfan