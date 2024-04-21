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

int file2msg(const std::string& filename, std::string &msg)
{
    std::ifstream ifile(filename.c_str());
    if (!ifile)
    {
      std::cerr << "Unable to read " << filename << std::endl;
      return -1;
    }

    msg = std::string((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
    ifile.close();
    return 0;
}

std::vector<std::string> execute_command(const std::string &command)
{
  std::vector<std::string> output;
  FILE *pipe = popen(command.c_str(), "r"); // Open the pipe for reading the command output
  if (!pipe)
  {
    std::cerr << "Could not execute command: " << command << std::endl;
    return output;
  }

  char buffer[128];
  while (fgets(buffer, 128, pipe) != NULL)
  {
    output.push_back(buffer);
  }

  int result = pclose(pipe); // Close the pipe and get the command result
  if (result == -1)
  {
    std::cerr << "Command execution failed." << std::endl;
  }

  return output;
}


} //namespace hefei