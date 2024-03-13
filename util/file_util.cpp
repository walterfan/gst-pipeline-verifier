#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include "string_util.h"
#include "file_util.h"
#include "yaml-cpp/yaml.h"

namespace wfan {


bool directory_exists(const std::string& directory) {
    struct stat info;
    if (stat(directory.c_str(), &info) != 0) // stat returns 0 on success
        return false;
    return (info.st_mode & S_IFDIR) != 0; // Check if it's a directory
}

bool file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good(); 
}

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

std::map<std::string, std::string> read_yaml_section(const std::string& file_path, 
    const std::string& section_path) {
    YAML::Node config = YAML::LoadFile(file_path);
    if (config[section_path]) {
        return config[section_path].as<std::map<std::string, std::string>>();
    }
    return std::map<std::string, std::string>();
}

int retrieve_files(const char* szFolder, std::vector<std::string>& files)
{
  struct dirent* direntp;
  DIR* dirp = opendir(szFolder);

  if(NULL == dirp) {
    return -1;
  }

  while( NULL != (direntp = readdir(dirp))) {
    std::string file = direntp->d_name;
    if(".." == file || "." == file)
        continue;
    files.push_back(file);
  }

  while((-1 == closedir(dirp)) && (errno == EINTR));

  return files.size();
}

} //namespace wfan