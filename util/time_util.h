/*
 * time_util.h
 *
 *  Created on: Jan 17, 2020
 *      Author: walter
 */

#ifndef WFAN_TIME_UTIL_H_
#define WFAN_TIME_UTIL_H_

#include <string>
#include <chrono>

constexpr auto ISO_8601_TIME_FMT = "%Y-%m-%dT%H:%M:%SZ";
constexpr auto DEFAUT_TIME_FMT = "%Y%m%d%H%M%S";
constexpr auto SHORT_TIME_FMT = "%y%m%d_%H%M%S";
constexpr auto SHORT_TIME_HOUR_FMT = "%y%m%d_%H";

namespace wfan {

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

std::string time_to_str(std::time_t time, const char* fmt = SHORT_TIME_FMT);

//format a timestamp as date-time format string
std::string get_current_time_str(const char* fmt = SHORT_TIME_FMT);

std::string timepoint_to_str(
    const std::chrono::system_clock::time_point& timePoint,
    const char* fmt = DEFAUT_TIME_FMT);

TimePoint str_to_timepoint(const std::string& strTime,
    const char* fmt = DEFAUT_TIME_FMT);


} /* namespace wfan */

#endif /* WFAN_TIME_UTIL_H_ */
