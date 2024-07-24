#include <iostream>
#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#include "pipeline_verifier.h"
#include "logger.h"
#include "string_util.h"
#include "file_util.h"
#include "web_console.h"
#include "command_line_parser.h"

using namespace hefei;

constexpr auto VERSION = "1.0.0";
constexpr auto USAGE = "-f <config_file> -p <pipeline_name> -d <debug_level> [-l -h -v -a]";

void usage(char *argv[])
{
    std::cout << "Usage: " << argv[0] << " " << USAGE << std::endl;
    std::cout << "- all arguments are optional" << std::endl;

    
    std::cout << "\t-d <log_level> : 0:trace, 1: debug, 2: info ..., default value set by config file" << std::endl;
    std::cout << "\t-f <config_file> : specify config file, it is ./etc/config.yaml by default" << std::endl;
    std::cout << "\t-h : this screen for usage help" << std::endl;
    std::cout << "\t-l : list pipelines" << std::endl;

    std::cout << "\t-p <pipeline_name> : specify a pipeline to start" << std::endl;
    std::cout << "\t-r <replace_variables> : specify variables value, e.g. a=b,c=d" << std::endl;
    std::cout << "\t-w <http_port>: enable web server on http_port" << std::endl;
}


int verify_pipeline(int argc, char *argv[], void* param=nullptr) {

    CommandLineParser parser(argc, argv);

    auto pipeline_name = parser.getOptionValue("p");
    
    auto verifier = std::make_unique<PipelineVerifier>(argc, argv);
    verifier->init(parser.getOptionValue("f"), parser.getOptionValue("d"));

    //handle arguments
    if (parser.hasOption("v")) {
        std::cout << "Version: " << VERSION << std::endl;
        return 0;
    }

    if (parser.hasOption("h")) {
        usage(argv);
        return 0;
    }

    if (parser.hasOption("l")) {
        verifier->list_pipelines(pipeline_name);
        return 0;
    }

    auto web_port = parser.getOptionValue("w");
    verifier->fork_web_server(str_to_int(web_port), parser.hasOption("w"));

    auto variables = parser.getOptionValue("r");

    if (!pipeline_name.empty()) {
        return verifier->run_pipeline(pipeline_name, variables);
    }

    usage(argv);
    return 0;
}

int main (int argc, char *argv[])
{
    #if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
    return gst_macos_main(verify_pipeline, argc, argv, NULL);
    #else
    return verify_pipeline(argc, argv);
    #endif
}
