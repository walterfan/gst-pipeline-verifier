#include "string_util.h"
#include <iomanip>

namespace hefei {

bool endswith(const std::string& filename, const std::string& suffix) {
    if (filename.length() >= suffix.length()) {
        return 0 == filename.compare (filename.length() 
        - suffix.length(), suffix.length(), suffix);
    } else {
        return false;
    }
}

bool startswith(const std::string& filename, const std::string& prefix) {
    
    if (filename.length() >= prefix.length()) {
        return 0 == filename.compare(0, prefix.size(), prefix);
    } else {
        return false;
    }
}

std::string trim_prefix_suffix(std::string& filename, 
    const std::string& prefix, 
    const std::string& suffix) {
    if (startswith(filename, prefix) && endswith(filename, suffix)) {
        return filename.substr(prefix.length(), 
            filename.length() - prefix.length() - suffix.length());
    }
    return filename;
}

std::string substr_no_prefix(std::string& filename, 
    const std::string& prefix, uint32_t len) {
    if (startswith(filename, prefix)) {
        return filename.substr(prefix.length(), len);
    }
    return filename;
}

int find_closed_smaller_value(std::vector<std::string>& numbers, const std::string& bigger_value) {
    if (numbers.empty()) {
        throw std::runtime_error("The list is empty.");
    }

    int64_t diff = stoull(bigger_value) - stoull(numbers[0]); 
    
    if (diff < 0) {
        return -1;
    } else if (diff == 0) {
        return 0;
    }

    int i = 1;
      
    for (; i < numbers.size(); ++i) {
        diff = stoull(bigger_value) - stoull(numbers[i]);  

        if (diff > 0) {
            continue;
        } else if (diff == 0) {
            return i;
        } else if (diff < 0) {
            return i -  1;
        }
    }
    return i;
}


int find_closed_bigger_value(std::vector<std::string>& numbers, const std::string& smaller_value) {
    if (numbers.empty()) {
        throw std::runtime_error("The list is empty.");
    }
    size_t size = numbers.size();
    int64_t diff = stoull(numbers[size - 1]) -  stoull(smaller_value); 
    
    if (diff < 0) {
        return -1;
    } else if (diff == 0) {
        return size - 1;
    }

    int i = size  - 2;
      
    for (; i >= 0; --i) {
        diff = stoull(numbers[i]) -  stoull(smaller_value);  

        if (diff > 0) {
            continue;
        } else if (diff == 0) {
            return i;
        } else if (diff < 0) {
            return i + 1;
        }
    }
    return i;
}


std::string str_tolower(std::string s)
{
    for(char &c : s)
        c = tolower(c);
    return s;
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && (*it == '.' || std::isdigit(*it))) ++it;
    return !s.empty() && it == s.end();
}

int str_to_int(const std::string& s) {
    if(is_number(s)) return stoi(s);
    return -1;
}
/*
std::string int_to_binary(int n) {
    std::bitset<sizeof(int) * 8> binary(n);
    return binary.to_string();
}
*/

std::string int_to_binary(int n) {
  std::string binary;
  
  while (n > 0) {
    binary = std::to_string(n % 2) + binary; 
    n = n / 2;
  }
  
  return binary;
}

int binary_to_int(const std::string& binary_str) {
    return std::stoi(binary_str, nullptr, 2);
}

std::string trim(const std::string& str, const std::string& whitespace)
{
    std::string::size_type strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    std::string::size_type strEnd = str.find_last_not_of(whitespace);
    std::string::size_type strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::string ltrim(const std::string& str, const std::string& whitespace)
{
    std::string::size_type strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content


    std::string::size_type strRange = str.size() - strBegin + 1;
    return str.substr(strBegin, strRange);
}

std::string rtrim(const std::string& str, const std::string& whitespace)
{
    std::string::size_type strEnd = str.find_last_not_of(whitespace);
    if (strEnd == std::string::npos)
        return ""; // no content

    std::string::size_type strRange = strEnd + 1;

    return str.substr(0, strRange);
}

int tokenize(const std::string& input, std::vector<std::string>& tokens) {
    bool is_in_quotation = false;
    bool is_delimiter = false;

    size_t input_length = input.length();
    if (input_length == 0) {
        return 0;
    }

    std::ostringstream oss;
    int count = 0;
    for(size_t i = 0 ; i < input_length; ++i) {
        char c = input[i];
        oss << c;
      
        if (c == '"' || c == '\'') {
            if (!is_in_quotation) {
                is_in_quotation = true;
            } else {
                is_in_quotation = false;
            } 
            continue;
        } 
        if(c == ' ' || c == ',') {
            is_delimiter = true;
            if (is_in_quotation) {
                continue;
            }
            auto token = oss.str();
            count ++;
            tokens.push_back(trim(token, " ,"));
            oss = std::ostringstream();
        } else {
            is_delimiter = false;
        } 
    }

    auto left_str = oss.str();
    if (!left_str.empty()) {
        count ++;
        tokens.push_back(trim(left_str, " ,"));
    }
    return count;
}

bool has_option(
    const std::vector<std::string_view>& args, 
    const std::string_view& option_name) {
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            return true;
    }
    
    return false;
}

std::string_view get_option(
    const std::vector<std::string_view>& args, 
    const std::string_view& option_name) {
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            if (it + 1 != end)
                return *(it + 1);
    }
    
    return "";
}

int split(std::string strValue, std::string separator, std::vector<std::string>& strArr) {
    if(strValue.empty()) return 0;

    std::string::size_type pos0 = 0;
    std::string::size_type pos1 = strValue.find(separator, pos0);
    std::string::size_type seplen = separator.size();

    int cnt = 0;
    while(pos1 != std::string::npos) {
        strArr.push_back(strValue.substr(pos0, pos1-pos0));
        cnt++;
        pos0 = pos1 + seplen;
        pos1 = strValue.find(separator, pos0);
    }

    if(pos1 > pos0) {
        strArr.push_back(strValue.substr(pos0, pos1 - pos0));
        cnt++;
    }

    return cnt;
}

std::string bytesToHex(uint8_t* bytes, size_t len) {
    std::stringstream ss;

    // Ensure the output is in uppercase and has two characters for each byte
    ss << std::uppercase << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << std::hex << static_cast<int>(bytes[i]);
    }

    return ss.str();
}

}//namespace hefei