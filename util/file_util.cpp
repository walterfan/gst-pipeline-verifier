#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include "string_util.h"
#include "file_util.h"
#include "yaml-cpp/yaml.h"

namespace hefei {


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

} //namespace hefei