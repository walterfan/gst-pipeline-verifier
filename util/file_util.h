#pragma once

#include <string>
#include <vector>
#include <map>

namespace hefei {

using config_map_t = std::map<std::string, std::string>;

bool directory_exists(const std::string& directory);

bool file_exists(const std::string& filename);

config_map_t read_yaml_section(const std::string& file_path, 
    const std::string& section_path);

int retrieve_files(const char* szFolder, std::vector<std::string>& files);

} //namespace hefei