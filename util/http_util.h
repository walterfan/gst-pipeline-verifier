#pragma once

#include <string>


namespace hefei {

int post_json(const std::string& url, const std::string& post_body, const std::string& post_header,
    long& response_code, std::string& response_body) ;

char const *http_response_status(int code);
}