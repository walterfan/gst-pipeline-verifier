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

class Logger
{
public:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    static Logger& get_instance();

    std::shared_ptr<spdlog::logger>& get_logger();

    void init(const std::string& folder, const std::string& name, int file_size, int file_count);
    void reset_level(int level, int flush_level, int flush_interval);

private:
    std::shared_ptr<spdlog::logger> m_logger;
};


#define DLOG(...) SPDLOG_LOGGER_DEBUG(Logger::get_instance().get_logger(), __VA_ARGS__)
#define ILOG(...) SPDLOG_LOGGER_INFO(Logger::get_instance().get_logger(), __VA_ARGS__)
#define WLOG(...) SPDLOG_LOGGER_WARN(Logger::get_instance().get_logger(), __VA_ARGS__)
#define ELOG(...) SPDLOG_LOGGER_ERROR(Logger::get_instance().get_logger(), __VA_ARGS__)




