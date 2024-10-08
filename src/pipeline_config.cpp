#include "gst_util.h"
#include "file_util.h"
#include "string_util.h"
#include "pipeline_config.h"
#include "logger.h"

using namespace std;

static const std::string DOT = ".";

namespace hefei {

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


PipelineConfig::PipelineConfig(const std::string& name, std::vector<std::string> elements,
    const std::string &desc, const std::string &tags)
: m_name(name)
, m_desc(desc)
, m_tags(tags)
, m_steps(elements)
{

}

std::shared_ptr<ElementConfig> PipelineConfig::get_element_config(const std::string& name) {
    for(auto& ele_cfg_ptr: m_elements_config) {
        if (ele_cfg_ptr->m_name == name) {
            return ele_cfg_ptr;
        }
    }
    return nullptr;
}

int PipelineConfig::clean() {
    if (!m_initialized) {
        return 0;
    }
    m_elements_config.clear();
    m_initialized = false;
    return 0;
}

int PipelineConfig::init(const std::string &variables)
{
    if (m_initialized) {
        return 0;
    }
    std::map<std::string, std::string> var_map;
    int count = split(variables, ",", "=", var_map);

    for (std::string &desc : m_steps)
    {
        if (count > 0) {

            int cnt = replace_variables(desc, var_map);
            if (cnt > 0) {
                DLOG("replace desc to {}", desc);
            }

        }
        m_elements_config.push_back(std::make_shared<ElementConfig>(desc));
    }
    m_initialized = true;
    return 0;
}

int PipelineConfig::check_elements_name()
{
    size_t count = m_elements_config.size();

    size_t i = 0, j = 0;

    for (; i < count - 1; ++i)
    {
        auto &ele_cfg_ptr = m_elements_config[i];
        auto &element_name = ele_cfg_ptr->m_name;
        size_t index = 0;
        for (j = i + 1; j < count; ++j)
        {
            auto &next_ptr = m_elements_config[j];
            auto &next_name = next_ptr->m_name;
            if (next_name == element_name)
            {
                next_name = element_name + std::to_string(++index);
                DLOG("{} rename to {}", element_name, next_name);
            }
        }
    }
    return 0;
}

std::string PipelineConfig::dump_pipeline_line() {
    std::ostringstream ss;
    int cnt = m_steps.size();
    for (int i = 0; i < cnt; ++i)
    {
        if (i > 0)
        {
            ss << " \\\n<br/>  ! ";
        }
        else
        {
            ss << "  gst-launch-1.0 ";
        }

        ss << m_steps[i];

    }
    return ss.str();
}

// --- probe config ---
bool ProbeConfig::has_probe_config_item(const std::string &item_name)
{
    return config_items.find(item_name) != config_items.end();
}

ProbeConfigItem &ProbeConfig::get_probe_config_item(const std::string &item_name)
{
    return config_items[item_name];
}

void ProbeConfig::add_probe_config_item(const ProbeConfigItem &probeConfigItem)
{
    config_items[probeConfigItem.probe_pipeline_name] = probeConfigItem;
}

// --- props config ----

bool PropsConfig::has_prop_config_item(const std::string &prop_key)
{
    return config_items.find(prop_key) != config_items.end();
}

PropConfigItem &PropsConfig::get_prop_config_item(const std::string &prop_key)
{
    return config_items[prop_key];
}

void PropsConfig::add_prop_config_item(const PropConfigItem &propConfigItem)
{
    config_items[propConfigItem.prop_key] = propConfigItem;
}

std::vector<std::string> PropsConfig::get_prop_keys(const std::string& pipeline_name) {
    std::vector<std::string> keys;


    return keys;

}
// --- pipeline config ---

PipelineConfigPtr PipelinesConfig::emplace(const std::string &name, std::vector<std::string> &steps,
                                           const std::string &desc, const std::string &tags) {
    auto pipeline_config_ptr = std::make_shared<PipelineConfig>(name, steps, desc, tags);
    m_pipelines.emplace(name, pipeline_config_ptr);
    return pipeline_config_ptr;
}

PipelineConfigPtr PipelinesConfig::get_pipeline_config(const std::string &name) {
    auto it = m_pipelines.find(name);
    if (it == m_pipelines.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace hefei
