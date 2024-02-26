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
constexpr auto CONFIG_FILE = "./etc/config.yaml";
constexpr auto VERSION = "1.0.0";
constexpr auto USAGE = "-f [config_file] -p [pipeline_name] [-h -v]";
constexpr auto KEY_GENERAL = "general";
constexpr auto KEY_LOG_FOLDER = "log_folder";
constexpr auto KEY_LOG_NAME   = "log_name";
constexpr auto KEY_LOG_LEVEL  = "log_level";


struct LogConfig {
    int log_level;
    std::string log_folder;
    std::string log_name;
    int load_config(const std::string& config_file);
};
// read config file
int LogConfig::load_config(const std::string& config_file) {
    auto general_config = read_section(std::string(config_file), KEY_GENERAL);
    if (general_config.empty()) {
        log_level = 2;
        log_folder = "./log";
        log_name = "pipeline_verify";
        return 1;
    }

    log_level = std::stoi(general_config[KEY_LOG_LEVEL]);
    log_folder = general_config[KEY_LOG_FOLDER];
    log_name   = general_config[KEY_LOG_NAME];
    return 0;
}


int verify_pipeline(int argc, char *argv[], void* param=nullptr) {
    
    const std::vector<std::string_view> args(argv, argv + argc);
    auto config_file = get_option(args, "-f");
    if (config_file.empty()) {
        std::cerr << "Use default configuration " << CONFIG_FILE << std::endl;
        config_file = CONFIG_FILE;
    }
    LogConfig logConfig;
    logConfig.load_config(std::string(config_file));
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
        return 0;
    }

    int ret = 0;
    auto verifier = std::make_unique<PipelineBuilder>(std::string(config_file));
    
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
