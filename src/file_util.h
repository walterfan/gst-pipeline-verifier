#pragma once

#include <string>
#include <vector>
#include <map>

int yaml_to_str_vec_map(const std::string& path, 
    const std::string& key1, 
    std::map<std::string, std::vector<std::string>>& config);



