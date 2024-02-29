/*
 * time_util.cpp
 *
 *  Created on: Jan 17, 2020
 *      Author: walter
 */

#include "time_util.h"

#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time

namespace wfan {

std::string time_to_str(std::time_t time, const char* fmt) {
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), fmt);
    return ss.str();
}

std::string get_current_time_str(const char* fmt) {
    auto now = std::chrono::system_clock::now();
    return timepoint_to_str(now, fmt);
}

std::string timepoint_to_str(
    const std::chrono::system_clock::time_point& timePoint,
    const char* fmt) {
    auto in_time_t = std::chrono::system_clock::to_time_t(timePoint);

    return time_to_str(in_time_t, fmt);
}

TimePoint str_to_timepoint(const std::string& strTime, const char* fmt) {
    std::istringstream ss(strTime);
    std::tm timeInfo = {};
    ss >> std::get_time(&timeInfo, fmt);

    return std::chrono::system_clock::from_time_t(std::mktime(&timeInfo));
}

} /* namespace wfan */
