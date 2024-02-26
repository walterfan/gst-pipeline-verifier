#include "gst_util.h"
#include "file_util.h"
#include "string_util.h"
#include "pipeline_config.h"
#include "logger.h"

using namespace std;

namespace wfan {

ElementConfig::ElementConfig(const std::string& desc) {
    parse_desc(desc);
}

void ElementConfig::set_factory(const std::string& factory) {
    m_factory = factory;
    trim_space(m_factory);
    if (m_name.empty()) {
        m_name = m_factory;
    }
}


void ElementConfig::insert_prop(const std::string& key, const std::string& value) {
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
        if (m_factory.empty()) {
            set_factory(token);
        } else {
            //stop and start another branch
            if (token.find(".") != string::npos) {
                m_fork_tag = trim(token, " \t.");
                DLOG("fork tag: {}", m_fork_tag);
            } else {
                WLOG("unknown token: {}", token);
            }
        }

    }
}

void ElementConfig::parse_desc(const std::string& desc) {

    std::vector<std::string> tokens;
    int count = tokenize(desc, tokens);

    if (count > 0) {
        set_factory(tokens[0]);
        DLOG("m_factory: {}", m_factory);
    }
    if (count == 1) {
        return;
    }

    for (int i=1; i < count; ++i) {
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

void PipelineConfig::insert_desc(const std::string& desc) {
    m_elements_desc.push_back(desc);
    m_elements_config.push_back(std::make_shared<ElementConfig>(desc));
}

std::shared_ptr<ElementConfig> PipelineConfig::get_element_config(const std::string& name) {
    for(auto& ele_cfg_ptr: m_elements_config) {
        if (ele_cfg_ptr->m_name == name) {
            return ele_cfg_ptr;
        }
    }
    return nullptr;
}
} //namespace wfan