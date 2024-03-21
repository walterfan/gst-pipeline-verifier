#include "command_line_parser.h"

namespace hefei {

CommandLineParser::CommandLineParser(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.substr(0, 2) == "--") {
            // Long option
            std::string key = arg.substr(2);
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options[key] = argv[++i];
            } else {
                options[key] = ""; // Flag without value
            }
        } else if (arg[0] == '-') {
            // Short option
            if (arg.size() > 2 && arg[2] == '=') {
                // Short option with value attached (e.g., -o=value)
                std::string key = arg.substr(1, 1);
                options[key] = arg.substr(3);
            } else {
                // Short option with separate value (e.g., -o value)
                std::string key = arg.substr(1, 1);
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    options[key] = argv[++i];
                } else {
                    options[key] = ""; // Flag without value
                }
            }
        } else {
            // Positional argument
            positionalArgs.push_back(arg);
        }
    }
}

bool CommandLineParser::hasOption(const std::string& option) const {
    return options.find(option) != options.end();
}

std::string CommandLineParser::getOptionValue(const std::string& option) const {
    if (hasOption(option)) {
        return options.at(option);
    } else {
        return ""; // Option not found
    }
}

const std::vector<std::string>& CommandLineParser::getPositionalArgs() const {
    return positionalArgs;
}


/*
int main(int argc, char* argv[]) {
    CommandLineParser parser(argc, argv);

    // Example usage:
    if (parser.hasOption("help")) {
        std::cout << "Help message goes here..." << std::endl;
    }

    std::string filename = parser.getOptionValue("filename");
    if (!filename.empty()) {
        std::cout << "Filename: " << filename << std::endl;
    }

    std::string output = parser.getOptionValue("o");
    if (!output.empty()) {
        std::cout << "Output: " << output << std::endl;
    }

    const std::vector<std::string>& positionalArgs = parser.getPositionalArgs();
*/
} //namespace hefei 