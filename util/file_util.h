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

int retrieve_files(std::string const &folder,
    std::string const &suffix,
    std::vector<std::string> &files);

int file2msg(const std::string &filename, std::string &msg);

std::vector<std::string> execute_command(const std::string &command);

} //namespace hefei