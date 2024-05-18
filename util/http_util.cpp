#include "http_util.h"

#include <curl/curl.h>
#include <iostream>
#include <map>
namespace hefei {

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int post_json(const std::string& url, const std::string& post_body, const std::string& post_header, long& response_code, std::string& response_body) {
    CURL *curl = nullptr;
    CURLcode res = CURLE_OK;

    curl_global_init(CURL_GLOBAL_ALL);

    // Initialize a curl handle
    curl = curl_easy_init();
    if (curl) {
        // The URL for the POST request
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // The data to send (the JSON payload)
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body.c_str());

        // Specify the POST method
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

        // Set the content type to application/json
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        if (!post_header.empty()) {
            headers = curl_slist_append(headers, post_header.c_str());
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set up the callback function to write the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
        } else {
            // Now, the responseString contains the response from the server
            res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            //std::cout << "Response:" << response_code << ", "<< response_body << " ." << std::endl;
        }

        // Clean up the headers
        if (headers) {
            curl_slist_free_all(headers);
        }

        // Always cleanup
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return (int)res;
}

char const *http_response_status(int code)
{
    char const *http_status = "unknown";
    static std::map<int, char const *> code_to_status = {
        {100, "Continue"},
        {101, "Switching Protocols"},
        {102, "Processing"},
        {103, "Early Hints"},
        {200, "OK"},
        {201, "Created"},
        {202, "Accepted"},
        {203, "Non-Authoritative Information"},
        {204, "No Content"},
        {205, "Reset Content"},
        {206, "Partial Content"},
        {207, "Multi-Status"},
        {208, "Already Reported"},
        {226, "IM Used"},
        {300, "Multiple Choices"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {303, "See Other"},
        {304, "Not Modified"},
        {305, "Use Proxy"},
        {306, "unused"},
        {307, "Temporary Redirect"},
        {308, "Permanent Redirect"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {402, "Payment Required"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {406, "Not Acceptable"},
        {407, "Proxy Authentication Required"},
        {408, "Request Timeout"},
        {409, "Conflict"},
        {410, "Gone"},
        {411, "Length Required"},
        {412, "Precondition Failed"},
        {413, "Payload Too Large"},
        {414, "URI Too Long"},
        {415, "Unsupported Media Type"},
        {416, "Range Not Satisfiable"},
        {417, "Expectation Failed"},
        {418, "I'm a teapot"},
        {421, "Misdirected Request"},
        {422, "Unprocessable Content"},
        {423, "Locked"},
        {424, "Failed Dependency"},
        {425, "Too Early"},
        {426, "Upgrade Required"},
        {428, "Precondition Required"},
        {429, "Too Many Requests"},
        {431, "Request Header Fields Too Large"},
        {451, "Unavailable For Legal Reasons"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
        {504, "Gateway Timeout"},
        {505, "HTTP Version Not Supported"},
        {506, "Variant Also Negotiates"},
        {507, "Insufficient Storage"},
        {508, "Loop Detected"},
        {510, "Not Extended"},
        {511, "Network Authentication Required"},
    };
    auto it = code_to_status.find(code);
    if (it != code_to_status.end()) {
        http_status = it->second;
    }
    return http_status;
}

}  // end of namespace
