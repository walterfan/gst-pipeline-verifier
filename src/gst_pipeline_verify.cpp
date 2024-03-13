#include <iostream>
#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "pipeline_builder.h"
#include "logger.h"
#include "string_util.h"
#include "file_util.h"

using namespace wfan;
    
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
    
    const std::vector<std::string_view> args(argv, argv + argc);
    std::string config_file = std::string(get_option(args, "-f"));
    if (config_file.empty()) {
        if (file_exists(CONFIG_FILE)) {
            config_file = CONFIG_FILE;
        } else {
            config_file = CONFIG_FOLDER;
            config_file.append("/");
            config_file.append(CONFIG_FILE);
        }        
    }
    
    Logger::get_instance().load_config(config_file);
    LogConfig logConfig = Logger::get_instance().get_log_config();
    std::string log_level = std::string(get_option(args, "-d"));
    if (!log_level.empty()) {
        logConfig.log_level = std::stoi(log_level);
    }
    //init logger
    auto& logger = Logger::get_instance();
    logger.init(logConfig.log_folder, logConfig.log_name, 20, 20);
    logger.reset_level(logConfig.log_level, SPDLOG_LEVEL_ERROR, SPDLOG_FLUSH_SEC);
    
    //handle arguments
    if (has_option(args, "-v")) {
        std::cout << "Version: " << VERSION << std::endl;
        return 0;
    }

    if (has_option(args, "-h")) {
        std::cout << "Usage: " << argv[0] << " " << USAGE << std::endl;
        std::cout << "- all arguments are optional" << std::endl;        
        std::cout << "\t-l : list pipelines" << std::endl;        
        std::cout << "\t-h : this screen for usage help" << std::endl;
        std::cout << "\t-a : load all config files under ./etc folder" << std::endl;
        std::cout << "\t-f <config_file> : specify config file, it is ./etc/config.yaml by default" << std::endl;
        std::cout << "\t-p <pipeline_name> : specify a pipeline to start" << std::endl;
        std::cout << "\t-d <log_level> : 0:trace, 1: debug, 2: info ..., default value set by config file" << std::endl;
        return 0;
    }

    int ret = 0;
    auto verifier = std::make_unique<PipelineBuilder>();
    verifier->read_config_file(config_file.c_str());

    if (has_option(args, "-a")) {
        verifier->read_all_config_files(CONFIG_FOLDER);
    }

    if (has_option(args, "-l")) {
        verifier->list_pipelines(std::string(get_option(args, "-p")));
        return 0;
    }

    ret = verifier->init(argc, argv, args);
    CHECK_VALUE("pipeline init, ret={}",  ret, 0);
    ret = verifier->build();
    CHECK_VALUE("pipeline build, ret={}", ret, 0);
    ret = verifier->start();
    CHECK_VALUE("pipeline start, ret={}", ret, 0);
    ret = verifier->stop();
    CHECK_VALUE("pipeline stop, ret={}",  ret, 0);
    ret = verifier->clean();
    CHECK_VALUE("pipeline clean, ret={}", ret, 0);
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
