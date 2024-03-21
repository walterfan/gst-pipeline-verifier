#include "logger.h"
#include <unistd.h>
#include <iostream>
#include "file_util.h"

using namespace spdlog;
using namespace sinks;

namespace hefei {


Logger& Logger::get_instance()
{
    static Logger instance;
    return instance;
}

static std::string get_folder_name(const std::string& filePath) {
    size_t found = filePath.find_last_of("/\\"); 
    if (found != std::string::npos) {
        return filePath.substr(0, found); 
    }
    return ""; 
}

static std::string get_log_path(const std::string& folder, const std::string& name, const std::string& suffix)
{
    time_t t = time(nullptr);
    struct tm* timeinfo;
    timeinfo = localtime(&t);

    char path[256] = {0};;
    snprintf(path, 256, "%s/%s_%04d%02d%02d_%02d%02d%02d_%d%s", folder.c_str(), name.c_str(),
            1900 + timeinfo->tm_year, 1 + timeinfo->tm_mon, timeinfo->tm_mday,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, getpid(), suffix.c_str());
    return path;
}


void Logger::init(const std::string& folder, const std::string& name, int file_size, int file_count)
{
    // pattern
    std::string pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] (%!) %v");

    // console sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern(pattern);

    // file sink
    auto log_path = get_log_path(folder, name, ".log");
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_path.c_str(), 
        1024 * 1024 * file_size, file_count);
    file_sink->set_level(spdlog::level::debug);
    file_sink->set_pattern(pattern);
    
    // register sinks
    spdlog::sinks_init_list sinks{ console_sink, file_sink };
    m_logger = std::make_shared<spdlog::logger>(name, sinks);
    
    // set pattern, level and flush interval
    m_logger->set_pattern(pattern);
    m_logger->set_level(spdlog::level::trace);
    m_logger->flush_on(spdlog::level::err);
    spdlog::flush_every(std::chrono::seconds(3));
    
    // register logger
    spdlog::register_logger(m_logger);
    spdlog::set_default_logger(m_logger);
}

std::shared_ptr<spdlog::logger>& Logger::get_logger()
{
    return m_logger;
}

spdlog::level::level_enum int_to_log_level(int l)
{
    spdlog::level::level_enum level;
    switch (l)
    {
        case SPDLOG_LEVEL_TRACE:
            level = spdlog::level::trace;
            break;
        case SPDLOG_LEVEL_DEBUG:
            level = spdlog::level::debug;
            break;
        case SPDLOG_LEVEL_INFO:
            level = spdlog::level::info;
            break;
        case SPDLOG_LEVEL_WARN:
            level = spdlog::level::warn;
            break;
        case SPDLOG_LEVEL_ERROR:
            level = spdlog::level::err;
            break;
        case SPDLOG_LEVEL_CRITICAL:
            level = spdlog::level::critical;
            break;
        case SPDLOG_LEVEL_OFF:
            level = spdlog::level::off;
            break;
        default:
            level = spdlog::level::n_levels;
            break;
    }
    return level;
}

void Logger::reset_level(int level, int flush_level, int flush_interval)
{
    
    if (m_logger) {
        std::vector<sink_ptr> sinks = m_logger->sinks();
        auto it = sinks.begin();
        for (; it != sinks.end(); it++)
        {
            (*it)->set_level(int_to_log_level(level));
        }
        m_logger->set_level(int_to_log_level(level));
        m_logger->flush_on(int_to_log_level(flush_level));
    }
    spdlog::flush_every(std::chrono::seconds(flush_interval));
}

}//namespace hefei
