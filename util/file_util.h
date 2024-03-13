#pragma once

#include <string>
#include <vector>
#include <map>

namespace wfan {

using config_map_t = std::map<std::string, std::string>;
using pipeline_config_t = std::map<std::string, std::vector<std::string>>;

bool directory_exists(const std::string& directory);

bool file_exists(const std::string& filename);

int yaml_to_str_vec_map(const std::string& file_path, 
    const std::string& node_path, 
    pipeline_config_t& config);


config_map_t read_yaml_section(const std::string& file_path, 
    const std::string& section_path);

int retrieve_files(const char* szFolder, std::vector<std::string>& files);

} //namespace wfan