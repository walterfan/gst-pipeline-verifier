#include <iostream>
#include <fstream>
#include <dirent.h>
#include "string_util.h"
#include "file_util.h"
#include "yaml-cpp/yaml.h"

namespace wfan {

int yaml_to_str_vec_map(const std::string& path, 
    const std::string& key1, 
    std::map<std::string, std::vector<std::string>>& config) {
    
    YAML::Node rootNode = YAML::LoadFile(path);
    YAML::Node childNode = rootNode[key1];
    if (!childNode) {
        std::cerr << "not find node by " << key1 << std::endl;
        return -1;
    }

    YAML::const_iterator it=childNode.begin();     
    for (; it!=childNode.end(); ++it) {
        std::string key = it->first.as<std::string>();
        std::vector<std::string> vals = it->second.as<std::vector<std::string>>();
        config.emplace(std::make_pair(key, vals));
    }
    
    return 0;
}

std::map<std::string, std::string> read_section(const std::string& file_path, 
    const std::string& section_path) {
    YAML::Node config = YAML::LoadFile(file_path);
    if (config[section_path]) {
        return config[section_path].as<std::map<std::string, std::string>>();
    }
    return std::map<std::string, std::string>();
}

} //namespace wfan