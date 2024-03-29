#pragma once

#include <string>
#include <cstdint>
#include <sstream>
#include <vector>
#include <map>
#include <sstream>

namespace hefei {

using cmd_args_t = std::vector<std::string_view>;

bool endswith(const std::string& filename, const std::string& suffix);

bool startswith(const std::string& filename, const std::string& prefix);

int tokenize(const std::string& input, std::vector<std::string>& tokens);

std::string substr_no_prefix(std::string& filename, 
    const std::string& prefix, uint32_t len);

template<class T>
T str_to_num(const std::string timestr) {
    T value;
    std::istringstream iss(timestr);
    iss >> value;
    return value;
}

std::string str_tolower(std::string s);

bool is_number(const std::string& s);

int str_to_int(const std::string& s);

std::string int_to_binary(int n);

int binary_to_int(const std::string& binary_str);

int find_closed_smaller_value(std::vector<std::string>& numbers, const std::string& target);

int find_closed_bigger_value(std::vector<std::string>& numbers, const std::string& target);

std::string trim_prefix_suffix(std::string& filename, 
    const std::string& prefix, 
    const std::string& suffix);

std::string trim(const std::string& str, const std::string& whitespace = " \t" );

std::string ltrim(const std::string& str, const std::string& whitespace = " \t");

std::string rtrim(const std::string& str, const std::string& whitespace = " \t");

inline std::string &trim_space(std::string &str)
{
    const char *spaces = " \n\r\t";
    str.erase(str.find_last_not_of(spaces) + 1);
    str.erase(0, str.find_first_not_of(spaces));
    return str;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
    trim_space(s);
    return s;
}

int split(const std::string& strValue, const std::string& separator, std::vector<std::string>& strArr);

int split(const std::string& strValue, const std::string& separator, std::pair<std::string, std::string> &pair);

int split(const std::string &strValue, const std::string &sep1, const std::string &sep2, std::map<std::string, std::string> &strMap);

std::string bytesToHex(uint8_t* bytes, size_t len);

int replace_variables(std::string &desc, std::map<std::string, std::string> &var_map);

} //namespace hefei