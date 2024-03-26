#include <iostream>
#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#include "pipeline_verifier.h"
#include "pipeline_builder.h"
#include "logger.h"
#include "string_util.h"
#include "file_util.h"
#include "web_console.h"
#include "command_line_parser.h"

using namespace hefei;
    
#define CHECK_VALUE(msg, value, expect)                          \
    do {                                                         \
        if ((value) != expect) {                                 \
            ELOG(msg, value);                                    \
            return value;                                        \
        } else {                                                 \
            ILOG(msg, value);                                    \
        }                                                        \
    } while (0)

constexpr auto SPDLOG_FLUSH_SEC = 3;
constexpr auto log_pattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] (%!) %v";
constexpr auto VERSION = "1.0.0";
constexpr auto USAGE = "-f <config_file> -p <pipeline_name> -d <debug_level> [-l -h -v -a]";

int verify_pipeline(int argc, char *argv[], void* param=nullptr) {
    
    CommandLineParser parser(argc, argv);

    std::string config_file = parser.getOptionValue("f");
    if (config_file.empty()) {
        if (file_exists(CONFIG_FILE)) {
            config_file = CONFIG_FILE;
        } else {
            config_file = CONFIG_FOLDER;
            config_file.append("/");
            config_file.append(CONFIG_FILE);
        }        
    }
    
    auto pipeline_name = parser.getOptionValue("p");
    auto web_port = parser.getOptionValue("w");
    
    auto verifier = std::make_unique<PipelineVerifier>(argc, argv);
    verifier->read_config_file(config_file.c_str());

    GeneralConfig& general_config = verifier->get_app_config().get_general_config();
    std::string log_level = parser.getOptionValue("d");
    if (!log_level.empty()) {
        general_config.log_level = std::stoi(log_level);
    }
    //init logger
    auto& logger = Logger::get_instance();
    logger.init(general_config.log_folder, general_config.log_name, 20, 20);
    logger.reset_level(general_config.log_level, SPDLOG_LEVEL_ERROR, SPDLOG_FLUSH_SEC);
    
    //handle arguments
    if (parser.hasOption("v")) {
        std::cout << "Version: " << VERSION << std::endl;
        return 0;
    }

    if (parser.hasOption("h")) {
        std::cout << "Usage: " << argv[0] << " " << USAGE << std::endl;
        std::cout << "- all arguments are optional" << std::endl;        
        std::cout << "\t-l : list pipelines" << std::endl;        
        std::cout << "\t-h : this screen for usage help" << std::endl;
        std::cout << "\t-w <http_port>: enable web server on http_port" << std::endl;
        std::cout << "\t-f <config_file> : specify config file, it is ./etc/config.yaml by default" << std::endl;
        std::cout << "\t-p <pipeline_name> : specify a pipeline to start" << std::endl;
        std::cout << "\t-d <log_level> : 0:trace, 1: debug, 2: info ..., default value set by config file" << std::endl;
        return 0;
    }

    if (directory_exists(CONFIG_FOLDER)) {
        verifier->read_all_config_files(CONFIG_FOLDER);
    }

    if (parser.hasOption("l")) {
        verifier->list_pipelines(pipeline_name);
        return 0;
    }

    if (parser.hasOption("w")) {
        general_config.http_enabled = 1;
    }
    std::unique_ptr<std::thread> thptr;
    if (general_config.http_enabled) {
        thptr = std::make_unique<std::thread>([&]{
            int http_port = -1;
            if (!web_port.empty()) {
                http_port = str_to_int(web_port);
            }
            if (http_port < 0) {
                http_port = general_config.http_port;
            }
            ILOG("start web server on {}", http_port);
            start_web_server(general_config.web_root.c_str(), http_port);
        });
    }
    auto builder = std::make_shared<PipelineBuilder>(verifier->get_app_config());
    int ret = builder->init(pipeline_name);
    CHECK_VALUE("pipeline init, ret={}",  ret, 0);
    ret = builder->build();
    CHECK_VALUE("pipeline build, ret={}", ret, 0);
    ret = builder->start();
    CHECK_VALUE("pipeline start, ret={}", ret, 0);
    ret = builder->stop();
    CHECK_VALUE("pipeline stop, ret={}",  ret, 0);
    ret = builder->clean();
    CHECK_VALUE("pipeline clean, ret={}", ret, 0);

    if (thptr) {
        thptr->join();
    }
    

    return ret;
}

int main (int argc, char *argv[])
{
    #if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
    return gst_macos_main(verify_pipeline, argc, argv, NULL);
    #else
    return verify_pipeline(argc, argv);
    #endif
}
