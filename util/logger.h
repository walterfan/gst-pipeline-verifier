#pragma once

#ifndef SPDLOG_TRACE_ON
#define SPDLOG_TRACE_ON
#endif

#ifndef SPDLOG_DEBUG_ON
#define SPDLOG_DEBUG_ON
#endif

#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include <string>
#include <memory>
#include <map>

namespace wfan {

struct LogConfig {
    // log_level: trace=0, debug=2, info=2, ...
    int log_level = 2;
    // GST_DEBUG_LEVEL: GST_LEVEL_FIXME=3, GST_LEVEL_INFO=4,..
    int debug_threshold = 3;
    std::string log_folder;
    std::string log_name;
};

class Logger
{
public:
    Logger()  = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    static Logger& get_instance();

    std::shared_ptr<spdlog::logger>& get_logger();

    void init(const std::string& folder, const std::string& name, int file_size, int file_count);
    void reset_level(int level, int flush_level, int flush_interval);

    int load_config(const std::string& config_file);
    LogConfig& get_log_config();
private:
    std::shared_ptr<spdlog::logger> m_logger;
    LogConfig m_log_config;
};

#define TLOG(...) SPDLOG_LOGGER_TRACE(Logger::get_instance().get_logger(), __VA_ARGS__)
#define DLOG(...) SPDLOG_LOGGER_DEBUG(Logger::get_instance().get_logger(), __VA_ARGS__)
#define ILOG(...) SPDLOG_LOGGER_INFO(Logger::get_instance().get_logger(), __VA_ARGS__)
#define WLOG(...) SPDLOG_LOGGER_WARN(Logger::get_instance().get_logger(), __VA_ARGS__)
#define ELOG(...) SPDLOG_LOGGER_ERROR(Logger::get_instance().get_logger(), __VA_ARGS__)

} //namespace wfan


