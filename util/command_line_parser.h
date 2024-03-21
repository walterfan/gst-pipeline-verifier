#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

namespace hefei {

class CommandLineParser {
public:
    CommandLineParser(int argc, char* argv[]);

    bool hasOption(const std::string& option) const ;

    std::string getOptionValue(const std::string& option) const ;

    const std::vector<std::string>& getPositionalArgs() const ;

private:
    std::unordered_map<std::string, std::string> options;
    std::vector<std::string> positionalArgs;
};

} //namespace hefei
